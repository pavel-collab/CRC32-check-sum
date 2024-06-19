#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/inotify.h>
#include <poll.h>
#include <string.h>

#include <algorithm>
#include <iostream>

#include "table.hpp"
#include "directory.hpp"
#include "crc32.hpp"
#include "Demon.hpp"

void handle_events(int fd) {

    /*  
    the structure inotify must be read all, 
    so it's necessary allocate enough buf size 
    */
    char buf[4096];
    const struct inotify_event *event;
    int i;
    ssize_t len;
    char *ptr;

    /* Loop while events can be read from inotify file descriptor. */
    for (;;) {

        /* read some events */
        len = read(fd, buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN) {
            perror("read");
            return;
        }

        if (len <= 0)
            break;

        //* man inotify(7) 
        for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
            event = (const struct inotify_event *) ptr;

            if (event->mask & IN_CREATE) {
                std::cout << "file was created" << std::endl;
                //TODO: raise the signal
            }

            if (event->mask & IN_DELETE) {
                std::cout << "file was deleted" << std::endl;
                //TODO: raise the signal
            }

            if (event->mask & IN_MODIFY){
                std::cout << "file was modiryed" << std::endl;
                //TODO: raise the signal
            }
        }
    }
}

void* threadInotifyRun(void* arg) {
    char* path_to_dir = (char*) arg;

    char buf;
    int fd, i, poll_num;
    int* wd;
    nfds_t nfds;

    struct pollfd fds[1];

    /* Create the file descriptor for accessing the inotify API */
    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1");
        return NULL;
    }

    /* Allocate memory for watch descriptors */
    wd = (int*) calloc(1, sizeof(int));
    if (wd == NULL) {
        perror("calloc");
        close(fd);
        return NULL;
    }

    wd[0] = inotify_add_watch(fd, path_to_dir, IN_CREATE | IN_DELETE | IN_MODIFY);
    if (wd[0] == -1) {
        fprintf(stderr, "Cannot watch %s\n", path_to_dir);
        perror("inotify_add_watch");
        close(fd);
        free(wd);
        return NULL;
    }

    /* Prepare for polling */
    nfds = 2;

    //* man poll(2)
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
                handle_events(fd);
            }
        }
    }

    /* Close inotify file descriptor */
    close(fd);
    free(wd);

    return NULL;
}

void* threadDemonRun(void* arg) {
    Demon* demon = (Demon*) arg;
    demon->startMainLoop();    
    return NULL;
}