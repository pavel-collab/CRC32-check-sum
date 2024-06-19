#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/inotify.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

#include <algorithm>
#include <iostream>

#include "table.hpp"
#include "directory.hpp"
#include "crc32.hpp"
#include "Demon.hpp"
#include "event_handler.hpp"

void handle_events(int fd, Demon* demon_ptr) {
    char buf[4096];
    const struct inotify_event *event;
    int i;
    ssize_t len;
    char *ptr;

    for (;;) {
        len = read(fd, buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN) {
            perror("read");
            return;
        }

        if (len <= 0)
            break;

        for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
            event = (const struct inotify_event *) ptr;

            if (event->mask & IN_CREATE) {
                Event* new_event_ptr = new AddFileEvent(event->name);
                demon_ptr->addEvent(new_event_ptr);
            }

            if (event->mask & IN_DELETE) {
                Event* new_event_ptr = new RmFileEvent(event->name);
                demon_ptr->addEvent(new_event_ptr);
            }

            if (event->mask & IN_MODIFY){
                Event* new_event_ptr = new CheckFileEvent(event->name);
                demon_ptr->addEvent(new_event_ptr);
            }
        }
    }
}

void* threadInotifyRun(void* arg) {
    Demon* demon = (Demon*) arg;

    char buf;
    int fd, i, poll_num;
    int* wd;
    nfds_t nfds = 2;

    struct pollfd fds[1];

    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1");
        return NULL;
    }

    wd = (int*) calloc(1, sizeof(int));
    if (wd == NULL) {
        perror("calloc");
        close(fd);
        return NULL;
    }

    wd[0] = inotify_add_watch(fd, demon->path_to_dir_.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY);
    if (wd[0] == -1) {
        perror("inotify_add_watch");
        close(fd);
        free(wd);
        return NULL;
    }

    fds[0].fd = STDIN_FILENO; // stdin
    fds[0].events = POLLIN; // watched event -- input

    fds[1].fd = fd; // inotify
    fds[1].events = POLLIN; // watched event -- input

    while (1) {
        poll_num = poll(fds, nfds, -1); // 2 -- amount of file descriptions; -1 -- infinity waiting time

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
                handle_events(fd, demon);
            }
        }
    }

    close(fd);
    free(wd);

    return NULL;
}

void* threadDemonRun(void* arg) {
    Demon* demon = (Demon*) arg;
    demon->startMainLoop();    
    return NULL;
}