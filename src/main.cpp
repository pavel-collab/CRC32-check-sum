#include <getopt.h>
#include <syslog.h>
#include <unistd.h>

#include <csignal>
#include <ctime>
#include <iostream>
#include <unordered_map>

#include "Daemon.hpp"
#include "Event.hpp"
#include "crc32.hpp"
#include "event_handler.hpp"
#include "table.hpp"
#include "syslogDump.hpp"

namespace {
// var for custom signal handler
volatile std::sig_atomic_t gSignalStatus;

void show_help() {
  printf("--help, -h\n"
         "\tPrint this message and exit\n"
         "\n"
         "--directory, -d\n"
         "\tPath to the directory daemon look after\n"
         "\n"
         "--periode, -p\n"
         "\tPeriode in sec for the Daemon\n"
         "\n"
         "--test, -t\n"
         "\tSet only for pytests to not to fork child proces\n");
}
} // namespace

static char *path_to_directory =
    NULL;        ///< path to the target directory; Daemon looks after this dir
int periode = 0; ///< periode of the automated check sum in seconds
int daemon_mode = 1; ///< this flag needs for the pytest

void signal_handler(int signal) {
  gSignalStatus = signal;
  if (signal == SIGALRM) {
    alarm(periode);
  }
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
      {NULL, 0, NULL, 0}};

  while ((res = getopt_long(argc, argv, short_opts, long_opts, &ind)) != -1)
    switch (res) {
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
      daemon_mode = 0;
      break;
    }
    }
}

int main(int argc, char *argv[]) {

  /*
  Parsing program arguments. By default program use command line arguments.
  If some of this args is not set, program use enviroment args: TARGET_DIR_PATH,
  PERIODE.
  */
  char *TARGET_DIR_PATH = getenv("TARGET_DIR_PATH");
  char *PERIODE = getenv("PERIODE");

  if (TARGET_DIR_PATH != NULL) {
    path_to_directory = TARGET_DIR_PATH;
  }

  if (PERIODE != NULL) {
    periode = atoi(PERIODE);
  }

  parse_args(argc, argv);

  if (periode <= 0) {
    SYSLOG_DUMP("[err] periode can't be less or equal 0, but actual periode is %d\n",
                periode);
    return -1;
  }

  if (path_to_directory == NULL) {
    SYSLOG_DUMP("[err] path to the directory is no set\n");
    return -1;
  }

  path_to_directory = realpath(path_to_directory, NULL);
  if (path_to_directory == NULL) {
    if (errno == ENOENT) {
      SYSLOG_DUMP("[err] target directory doesn't exist\n");
    } else {
      SYSLOG_DUMP("[err] unexpected problem with target directory\n");
    }
    return -1;
  }

  // Daemon; fork the child proces and let the parent proces go
  // (doesn't work in test mode)
  if (daemon_mode) {
    pid_t parpid;
    if ((parpid = fork()) < 0) {
      printf("\ncan't fork");
      exit(1);
    } else if (parpid != 0) {
      printf("Parent procwith pid %d\n", getpid());
      exit(0);
    }
    setsid(); // switch child proc in new session
  }

  // signal handle
  std::signal(SIGUSR1, signal_handler); // SIGUSR1 -- immediate check crc32 sums
  std::signal(
      SIGALRM,
      signal_handler); // SIGALRM -- using alarm to check crc32 by timeout
  std::signal(SIGTERM, signal_handler); // SIGTERM -- Daemon end of work

  // ignore this signals
  std::signal(SIGQUIT, signal_handler);
  std::signal(SIGINT, signal_handler);
  std::signal(SIGHUP, signal_handler);
  std::signal(SIGSTOP, signal_handler);
  std::signal(SIGCONT, signal_handler);

  alarm(periode);

  Daemon *new_daemon = Daemon::getInstance(path_to_directory); // create Daemon

  // create and run thread for inotify
  pthread_t inotify_thread;
  if (errno =
          pthread_create(&inotify_thread, NULL, threadInotifyRun, new_daemon)) {
    perror("pthread_create");
    return -1;
  }

  // create and run thread for Daemon
  pthread_t daemon_thread;
  if (errno =
          pthread_create(&daemon_thread, NULL, threadDaemonRun, new_daemon)) {
    perror("pthread_create");
    return -1;
  }

  // waiting for the some signal
  while (1) {
    sleep(1);
    
    if (gSignalStatus == SIGUSR1 || gSignalStatus == SIGALRM) {
      Event *new_event_ptr = new CheckSumEvent(path_to_directory);
      new_daemon->addEvent(new_event_ptr);
      gSignalStatus = -1;
    } else if (gSignalStatus == SIGTERM) {
      Event *new_event_ptr = new ExitEvent(path_to_directory);
      new_daemon->addEvent(new_event_ptr);
      gSignalStatus = -1;
      break;
    }
  }

  pthread_join(daemon_thread, NULL);

  return 0;
}