#include <unistd.h>
#include <getopt.h>
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
    // var for custom signal handler
    volatile std::sig_atomic_t gSignalStatus;
}

static char *path_to_directory = NULL; ///< path to the target directory; demon looks after this dir
int periode = 0; ///< periode of the automated check sum in seconds
int demon = 1; ///< this flag needs for the pytest


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
            "\tPeriode in sec for the demon\n"
            "\n"
            "--test, -t\n"
            "\tSet only for pytests to not to fork child proces\n");
}

/**
 * Parsing comand line arguments.
 */
static void parse_args(int argc, char **argv) {
    int res = 0, ind = 0;
    const char *short_opts = "hf:";
    const struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"directory", required_argument, NULL, 'd'},
        {"periode", required_argument, NULL, 'p'},
        {"test", no_argument, NULL, 't'},
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
            case 't': {
                demon = 0;
                break;
            }
        }
}

int main(int argc, char* argv[]) {

    /*
    Parsing program arguments. By default program use enviroment args: TARGET_DIR_PATH, PERIODE.
    If some of this enviroment args is not set, program use command line arguments.
    */
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
        //? move this 3 calls to one function
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
    if (path_to_directory == NULL) {
        if (errno == ENOENT) {
            openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
            syslog(LOG_INFO, "[err] target directory doesn't exist\n");
            closelog();
        } else {
            openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
            syslog(LOG_INFO, "[err] unexpected problem with target directory\n");
            closelog();
        }
        return -1;
    }

    // demon; fork the child proces and let the parent proces go
    // (doesn't work in test mode)
    if (demon) {
        pid_t parpid;
        if((parpid=fork())<0) {  
            printf("\ncan't fork"); 
            exit(1);                
        }
        else if (parpid != 0) {
            printf("Parent procwith pid %d\n", getpid());
            exit(0);
        }       
        setsid(); // switch child proc in new session
    }

    // signal handle
    std::signal(SIGUSR1, signal_handler); // SIGUSR1 -- immediate check crc32 sums
    std::signal(SIGALRM, signal_handler); // SIGALRM -- using alarm to check crc32 by timeout
    std::signal(SIGTERM, signal_handler); // SIGTERM -- demon end of work

    // ignore this signals
    std::signal(SIGQUIT, signal_handler);
    std::signal(SIGINT, signal_handler);
    std::signal(SIGHUP, signal_handler);
    std::signal(SIGSTOP, signal_handler);
    std::signal(SIGCONT, signal_handler);

    alarm(periode);

    Demon* new_demon = Demon::getInstance(path_to_directory); // create demon

    // create and run thread for inotify
    pthread_t inotify_thread;
    if (errno = pthread_create(&inotify_thread, NULL, threadInotifyRun, new_demon)) {
        perror("pthread_create");
        return -1;
    }

    // create and run thread for demon
    pthread_t demon_thread;
    if (errno = pthread_create(&demon_thread, NULL, threadDemonRun, new_demon)) {
        perror("pthread_create");
        return -1;
    }

    // waiting for the some signal
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

    pthread_join(demon_thread, NULL);

    return 0;
}