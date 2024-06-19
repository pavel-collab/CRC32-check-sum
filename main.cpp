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
#include <getopt.h>
#include <getopt.h>
#include <limits.h>
#include <sys/types.h>

#include <unordered_map>
#include <ctime>
#include <csignal>
#include <iostream>

#include "table.hpp"
#include "directory.hpp"
#include "crc32.hpp"
#include "event_handler.hpp"
#include "Demon.hpp"
#include "Event.hpp"

namespace {
    volatile std::sig_atomic_t gSignalStatus;
}

static char *path_to_directory = NULL;
int periode = 0;

void signal_handler(int signal) {
    gSignalStatus = signal;
    if (signal == SIGALRM) {
        alarm(periode);
    }
}

static void show_help() {
    printf ("--help, -h\n"
            "\tPrint this message and exit\n"
            "\n"
            "--directory, -d\n"
            "\tPath to the directory demon look after\n"
            "\n"
            "--periode, -p\n"
            "\tPeriode in sec for the demon\n");
}

static void parse_args (int argc, char **argv) {
    int res = 0, ind = 0;
    const char *short_opts = "hf:";
    const struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"directory", required_argument, NULL, 'd'},
        {"periode", required_argument, NULL, 'p'},
        {NULL, 0, NULL, 0}
    };

    while ((res = getopt_long(argc, argv, short_opts, long_opts, &ind)) != -1)
        switch (res)
        {
            default:
            case 'h': {
                show_help();
                return;
            }
            case 'd': {
                path_to_directory = optarg;
                break;
            }
            case 'p': {
                periode = atoi(optarg);
                break;
            }
        }
}

int main(int argc, char* argv[]) {

    parse_args(argc, argv);

    char* TARGET_DIR_PATH = getenv("TARGET_DIR_PATH");
    char* PERIODE = getenv("PERIODE");

    if (TARGET_DIR_PATH != NULL) {
        path_to_directory = TARGET_DIR_PATH;
    }

    if (PERIODE != NULL) {
        periode = atoi(PERIODE);
    }

    if (periode <= 0) {
        fprintf(stderr, "[err] the periode can't be <= 0, but it's %d\n", periode);
        return -1;
    }

    if (path_to_directory == NULL) {
        perror("path_to directory is still NULL");
        return -1;
    }

    path_to_directory = realpath(path_to_directory, NULL);

    std::signal(SIGUSR1, signal_handler);
    std::signal(SIGUSR2, signal_handler);
    std::signal(SIGURG, signal_handler);
    std::signal(SIGPROF, signal_handler);
    std::signal(SIGALRM, signal_handler);

    alarm(periode);

    // create and run thread for inotify
    inotifyThreadArgs inotify_thread_args{getpid(), path_to_directory};

    pthread_t inotify_thread;
    if (errno = pthread_create(&inotify_thread, NULL, threadInotifyRun, &inotify_thread_args)) {
        perror("pthread_create");
        return 1;
    }

    Demon* new_demon = Demon::getInstance(path_to_directory);
    // create and run thread for demon
    pthread_t demon_thread;
    if (errno = pthread_create(&demon_thread, NULL, threadDemonRun, new_demon)) {
        perror("pthread_create");
        return 1;
    }

    while(1) {
        sleep(1);
        if (gSignalStatus == SIGUSR1) {
            //TODO: проверить, нет ли здесь провисшего указателя
            Event* new_event_ptr = new RehashEvent{};
            new_demon->addEvent(new_event_ptr);
            gSignalStatus = -1;
        }

        if (gSignalStatus == SIGUSR2) {
             //TODO: проверить, нет ли здесь провисшего указателя
            Event* new_event_ptr = new RehashEvent{};
            new_demon->addEvent(new_event_ptr);
            gSignalStatus = -1;
        }

        if (gSignalStatus == SIGURG) {
            //TODO: проверить, нет ли здесь провисшего указателя
            Event* new_event_ptr = new DumpEvent{};
            new_demon->addEvent(new_event_ptr);
            gSignalStatus = -1;
        }

        if (gSignalStatus == SIGPROF) {
            //TODO: проверить, нет ли здесь провисшего указателя
            Event* new_event_ptr = new ExitEvent{};
            new_demon->addEvent(new_event_ptr);
            gSignalStatus = -1;
        }

        if (gSignalStatus == SIGALRM) {
            Event* new_event_ptr = new RehashEvent{};
            new_demon->addEvent(new_event_ptr);
            gSignalStatus = -1;
        }
    }

    return 0;
}