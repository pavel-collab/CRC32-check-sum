#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sched.h>
#include <poll.h>
#include <sys/inotify.h>
#include <pthread.h>

#include <unordered_map>
#include <ctime>

#include "table.hpp"
#include "directory.hpp"

// нельзя оптимизировать использование этой переменной
volatile int cought_signum;
volatile int proc_pid;

// прокачанный обработчик
void sig_handler(int signum, siginfo_t *info, void *ctx) {
    cought_signum = signum;
    if (cought_signum == SIGINT) {
        printf("[DEBUG]\n");
    }
    proc_pid = info->si_pid;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s filename to check sum\n", argv[0]);
        return -1;
    }

    const char* path_to_directory = argv[1];
    int period = 60;
    if (argc == 3) {
        period = atoi(argv[2]);
    }

    struct sigaction term_handler = {0};

    term_handler.sa_sigaction = sig_handler;
    term_handler.sa_flags = SA_RESTART | SA_SIGINFO;
    if (sigaction(SIGQUIT, &term_handler, NULL) == -1) {
        perror("sigaction(SIGTSTP)");
        return -1;
    } // ^/
    if (sigaction(SIGHUP, &term_handler, NULL) == -1) {
        perror("sigaction(SIGTSTP)");
        return -1;
    } // обрыв соединения
    if (sigaction(SIGTERM, &term_handler, NULL) == -1) {
        perror("sigaction(SIGTSTP)");
        return -1;
    } // завершение работы

    auto file_list = GetObjectList(path_to_directory);
    std::unordered_map<std::string, unsigned int> check_sum_container;

    pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
    Directory dir{1, period, mu, path_to_directory, check_sum_container};
    
    unsigned int check_sum = 0;
    std::string file_path;
    for (auto file : file_list) {
        file_path = std::string(path_to_directory) + "/" + file;
        check_sum = ChecSum(file_path.c_str());
        dir.check_sum_container.insert({file_path, check_sum});
        printf("0x%08x\n", check_sum);
    }
    
    pthread_t thread;
    if (errno = pthread_create(&thread, NULL, event_main_loop, &dir)) {
        //! прибивать потоки и чистить ресурсы в случае аварийной остановки программы
        perror("pthread_create");
        return 1;
    }

    char buf;
    int fd, i, poll_num;
    int* wd;
    nfds_t nfds;

    struct pollfd fds[1];

    /* Create the file descriptor for accessing the inotify API */
    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1");
        return -1;
    }

    /* Allocate memory for watch descriptors */
    wd = (int*) calloc(1, sizeof(int));
    if (wd == NULL) {
        perror("calloc");
        return -1;
    }

    wd[0] = inotify_add_watch(fd, path_to_directory, IN_CREATE | IN_DELETE);
    if (wd[0] == -1) {
        fprintf(stderr, "Cannot watch %s\n", argv[i]);
        perror("inotify_add_watch");
        return -1;
    }

    /* Prepare for polling */
    nfds = 2;

    //* man poll(2)
    fds[0].fd = STDIN_FILENO; // stdin
    fds[0].events = POLLIN; // watched event -- input

    fds[1].fd = fd; // inotify
    fds[1].events = POLLIN; // watched event -- input

    /* Wait for events and/or terminal input */
    printf("Waiting for events\n\n");
    while (1) {

        poll_num = poll(fds, nfds, -1); // 2 -- amount of file descriptions; -1 -- infinity waiting time

        if (poll_num == -1) {
            if (errno == EINTR)
                continue;
            perror("poll");
            return -1;
        }

        if (poll_num > 0) {

            if ((fds[0].revents & POLLIN)) {
                /* Console input is available. Empty stdin and quit */
                pause();
            }

            if (fds[1].revents & POLLIN) {
                /* Inotify events are available */
                puts("-------------------------------------------------------------------------------------------");
                handle_events(fd, &dir);
                puts("-------------------------------------------------------------------------------------------");
            }
        }
    }

    pthread_join(thread, NULL);

    /* Close inotify file descriptor */
    close(fd);
    free(wd);

    return 0;
}