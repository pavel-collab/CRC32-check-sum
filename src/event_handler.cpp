#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>

#include "Daemon.hpp"
#include "crc32.hpp"
#include "event_handler.hpp"
#include "table.hpp"

/**
 * Function read caught inotify events and generate sutable Event objects.
 * (man 7 inotify)
 *
 * @param fd -- inotify file descriptor
 * @param daemon_ptr -- pointer to the Daemon object (we need it to put Event
 * object in the event queue)
 */
void handle_events(int fd, Daemon *daemon_ptr) {
  char buf[4096];
  const struct inotify_event *event;
  int i;
  ssize_t len;
  char *ptr;

  // Loop while events can be read from inotify file descriptor
  for (;;) {

    // Read some events
    len = read(fd, buf, sizeof(buf));
    if (len == -1 && errno != EAGAIN) {
      perror("read");
      return;
    }

    if (len <= 0)
      break;

    // Loop over all events in the buffer
    for (ptr = buf; ptr < buf + len;
         ptr += sizeof(struct inotify_event) + event->len) {
      event = (const struct inotify_event *)ptr;

      if ((event->mask & IN_CREATE) && !(event->mask & IN_ISDIR)) {
        Event *new_event_ptr =
            new AddFileEvent(daemon_ptr->getTargetDirPath(), event->name);
        daemon_ptr->addEvent(new_event_ptr);
      }

      if ((event->mask & IN_DELETE) && !(event->mask & IN_ISDIR)) {
        Event *new_event_ptr =
            new RmFileEvent(daemon_ptr->getTargetDirPath(), event->name);
        daemon_ptr->addEvent(new_event_ptr);
      }

      if ((event->mask & IN_MODIFY) && !(event->mask & IN_ISDIR)) {
        Event *new_event_ptr =
            new CheckFileEvent(daemon_ptr->getTargetDirPath(), event->name);
        daemon_ptr->addEvent(new_event_ptr);
      }
    }
  }
}

void *threadInotifyRun(void *arg) {
  Daemon *daemon = (Daemon *)arg;

  // here we use poll to monitore inotify events (check man poll)
  char buf;
  int fd, i, poll_num;
  int *wd;
  nfds_t nfds = 2;

  struct pollfd fds[1];

  fd = inotify_init1(IN_NONBLOCK);
  if (fd == -1) {
    perror("inotify_init1");
    return NULL;
  }

  wd = new int[1];

  wd[0] = inotify_add_watch(fd, daemon->getTargetDirPath().c_str(),
                            IN_CREATE | IN_DELETE | IN_MODIFY);
  if (wd[0] == -1) {
    perror("inotify_add_watch");
    close(fd);
    free(wd);
    return NULL;
  }

  fds[0].fd = STDIN_FILENO; // stdin
  fds[0].events = POLLIN;   // watched event -- input

  fds[1].fd = fd;         // inotify
  fds[1].events = POLLIN; // watched event -- input

  while (1) {
    poll_num = poll(
        fds, nfds,
        -1); // 2 -- amount of file descriptions; -1 -- infinity waiting time

    if (poll_num == -1) {
      if (errno == EINTR)
        continue;
      perror("poll");
      close(fd);
      free(wd);
      return NULL;
    }

    if (poll_num > 0) {

      if ((fds[0].revents & POLLIN)) {
        // here we can catch message from stdin for example to exit program
      }

      if (fds[1].revents & POLLIN) {
        handle_events(fd, daemon);
      }
    }
  }

  close(fd);
  delete[] wd;

  return NULL;
}

void *threadDaemonRun(void *arg) {
  Daemon *daemon = (Daemon *)arg;
  daemon->startMainLoop();
  return NULL;
}