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
#include <syslog.h>

#include <unordered_map>
#include <ctime>
#include <csignal>
#include <iostream>

#include "table.hpp"
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
        //? Можно ли вынести в отдельную функцию?
        openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "[err] periode can't be less or equal 0, but actual periode is %d\n", periode);
        closelog();
        return -1;
    }

    if (path_to_directory == NULL) {
        openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "[err] path to the directory is no set\n");
        closelog();
        return -1;
    }

    path_to_directory = realpath(path_to_directory, NULL);

    std::signal(SIGUSR1, signal_handler);
    std::signal(SIGALRM, signal_handler);

    // сигнал для завершения работы демона
    std::signal(SIGTERM, signal_handler);

    // игнорируем эти сигналы
    std::signal(SIGQUIT, signal_handler);
    std::signal(SIGINT, signal_handler);
    std::signal(SIGHUP, signal_handler);
    std::signal(SIGSTOP, signal_handler);
    std::signal(SIGCONT, signal_handler);

    alarm(periode);

    Demon* new_demon = Demon::getInstance(path_to_directory);

    // create and run thread for inotify
    pthread_t inotify_thread;
    if (errno = pthread_create(&inotify_thread, NULL, threadInotifyRun, new_demon)) {
        //? error to syslog or to stderr
        perror("pthread_create");
        return -1;
    }

    // create and run thread for demon
    pthread_t demon_thread;
    if (errno = pthread_create(&demon_thread, NULL, threadDemonRun, new_demon)) {
        //? error to syslog or to stderr
        perror("pthread_create");
        return -1;
    }

    while(1) {
        sleep(1);
        if (gSignalStatus == SIGUSR1) {
            Event* new_event_ptr = new CheckSumEvent(path_to_directory);
            new_demon->addEvent(new_event_ptr);
            gSignalStatus = -1;
        }

        if (gSignalStatus == SIGALRM) {
            Event* new_event_ptr = new CheckSumEvent(path_to_directory);
            new_demon->addEvent(new_event_ptr);
            gSignalStatus = -1;
        }

        if (gSignalStatus == SIGTERM) {
            Event* new_event_ptr = new ExitEvent(path_to_directory);
            new_demon->addEvent(new_event_ptr);
            gSignalStatus = -1;
            break;
        }
    }

    return 0;
}