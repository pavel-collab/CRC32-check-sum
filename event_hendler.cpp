#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/inotify.h>
#include <poll.h>

#include <algorithm>

#include "table.hpp"
#include "directory.hpp"

void handle_events(int fd, Directory* dir) {

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
        /*  
        If the nonblocking read() found no events to read, then
        it returns -1 with errno set to EAGAIN. In that case,
        we exit the loop. 
        */

        if (len <= 0)
            break;

        //* man inotify(7) 
        for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
            event = (const struct inotify_event *) ptr;

            if (event->mask & IN_CREATE) {
                if (event->len && !(event->mask & IN_ISDIR)) {
                    std::string file_path = std::string(dir->path_to_directory) + "/" + event->name;
                    unsigned int check_sum = ChecSum(file_path.c_str());
                    dir->check_sum_container.insert({file_path, check_sum});
                    printf("0x%08x\n", check_sum);
                    
                    for (auto f : dir->check_sum_container) {
                        printf("[DEBUG] %s\n", f.first.c_str());
                    }
                }
            }

            if (event->mask & IN_DELETE) {
                if (event->len && !(event->mask & IN_ISDIR)) {
                    dir->check_sum_container.erase(event->name);
                    
                    for (auto f : dir->check_sum_container) {
                        printf("[DEBUG] %s\n", f.first.c_str());
                    }
                }
            }
        }
    }
}

void* event_main_loop(void* arg) {
    Directory* dir = (Directory*) arg;
    unsigned int cs = 0;

    while(1) {
        sleep(dir->periode);
        pthread_mutex_lock(&dir->mu);
        if (dir->flag == 0) {
            break;
        }
        pthread_mutex_unlock(&dir->mu);

        for (auto file : dir->check_sum_container) {
            pthread_mutex_lock(&dir->mu);
            cs = ChecSum(file.first.c_str());
            pthread_mutex_unlock(&dir->mu);
            if (cs != file.second) {
                fprintf(stderr, "[err] invalid check sum for file %s\n\tExpected 0x%08x, but got 0x%08x", file.first.c_str(), file.second, ChecSum(file.first.c_str()));
            }
        }
    }
    return NULL;
}