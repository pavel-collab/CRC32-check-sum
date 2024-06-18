#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/inotify.h>
#include <poll.h>
#include <string.h>

#include <algorithm>

#include "table.hpp"
#include "directory.hpp"
#include "crc32.hpp"

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
                }
            }

            if (event->mask & IN_DELETE) {
                if (event->len && !(event->mask & IN_ISDIR)) {
                    std::string file_path = std::string(dir->path_to_directory) + "/" + event->name;
                    auto pos = dir->check_sum_container.find(file_path);
                    if(pos != dir->check_sum_container.end()){
                        dir->check_sum_container.erase(pos);
                    }
                }
            }

            if (event->mask & IN_MODIFY){
                if (event->len && !(event->mask & IN_ISDIR)) {
                    std::string file_path = std::string(dir->path_to_directory) + "/" + event->name;
                    unsigned int crc_sum = ChecSum(file_path.c_str());
                    if (crc_sum != dir->check_sum_container[file_path]) {
                        fprintf(stderr, "[err] invalid check sum for file %s\n\tExpected 0x%08x, but got 0x%08x\n", file_path.c_str(), dir->check_sum_container[file_path], crc_sum);
                        dir->check_sum_container[file_path] = crc_sum;
                    }
                }
            }
        }
    }
}

void* event_main_loop(void* arg) {
    ThreadArgs* args = (ThreadArgs*) arg;
    int pipe = args->fd;
    while(1) {
        sleep(args->periode);
        write(pipe, "1", strlen("1")+1);
    }
    return NULL;
}