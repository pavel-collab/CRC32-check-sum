#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/inotify.h>

#include "table.hpp"

void handle_events(int fd, int *wd, int argc, char* argv[]) {

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
        /*  If the nonblocking read() found no events to read, then
            it returns -1 with errno set to EAGAIN. In that case,
            we exit the loop. */


        if (len <= 0)
            break;

        //* man inotify(7) 
        for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
            event = (const struct inotify_event *) ptr;

            /* print event type */
            // if (event->mask & IN_OPEN)
            //     printf("IN_OPEN:          ");
            // if (event->mask & IN_CLOSE_NOWRITE)
            //     printf("IN_CLOSE_NOWRITE: ");
            // if (event->mask & IN_CLOSE_WRITE)
            //     printf("IN_CLOSE_WRITE:   ");
            // if (event->mask & IN_ACCESS)
            //     printf("IN_ACCESS:        ");
            if (event->mask & IN_CREATE)
                printf("IN_CREATE:        ");
            if (event->mask & IN_DELETE)
                printf("IN_DELETE:        ");
            // if (event->mask & IN_MODIFY)
            //     printf("IN_MODIFY:        ");
            // if (event->mask & IN_ATTRIB)
            //     printf("IN_ATTRIB:        ");
            

            if (event->len)
                printf("%s", event->name);

            if (event->mask & IN_ISDIR)
                printf(" [directory]\n");
            else
                printf(" [file]\n");
            /*
            name of the system obj:
            if (event->len)
                printf("%s", event->name);

            type of the system obj:
            if (event->mask & IN_ISDIR)
                printf(" [directory]\n");
            else
                printf(" [file]\n");
            */
        }
    }
}