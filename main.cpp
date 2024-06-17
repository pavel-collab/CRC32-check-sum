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

#include <unordered_map>

#include "table.hpp"

// нельзя оптимизировать использование этой переменной
volatile int cought_signum;
volatile int proc_pid;

// прокачанный обработчик
void sig_handler(int signum, siginfo_t *info, void *ctx) {
    cought_signum = signum;
    proc_pid = info->si_pid;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s filename to check sum\n", argv[0]);
        return -1;
    }

    const char* path_to_file = argv[1];
    int period = 60;
    if (argc == 3) {
        period = atoi(argv[2]);
    }

    struct sigaction term_handler = {0};

    term_handler.sa_sigaction = sig_handler;
    term_handler.sa_flags = SA_RESTART | SA_SIGINFO;
    // if (sigaction(SIGTSTP, &term_handler, NULL) == -1) {
    //     perror("sigaction(SIGTSTP)");
    //     return -1;
    // } // ^z
    if (sigaction(SIGINT, &term_handler, NULL) == -1) {
        perror("sigaction(SIGTSTP)");
        return -1;
    } // ^c
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

    auto file_list = GetObjectList(path_to_file);
    std::unordered_map<std::string, unsigned int> check_sum_container;
    
    unsigned int check_sum = 0;
    std::string file_path;
    for (auto file : file_list) {
        file_path = std::string(path_to_file) + "/" + file;
        check_sum = ChecSum(file_path.c_str());
        check_sum_container.insert({file_path, check_sum});
        printf("0x%08x\n", check_sum);
    }

    int counter = 0;
    while(1) {
        sleep(period);

        for (auto file : check_sum_container) {
            if (ChecSum(file.first.c_str()) != file.second) {
                fprintf(stderr, "[err] invalid check sum for file %s\n\tExpected 0x%08x, but got 0x%08x", file.first.c_str(), file.second, ChecSum(file.first.c_str()));
            }
        }
    }

    return 0;
}