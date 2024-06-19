#ifndef _EVENT_HANDLDER_H_
#define _EVENT_HANDLDER_H_

#include <sys/types.h>

#include "directory.hpp"

struct inotifyThreadArgs {
    pid_t main_proc_pid;
    char* path_to_dir;
};

void handle_events(int fd);

void* threadInotifyRun(void* arg);

void* threadDemonRun(void* arg);

#endif //_EVENT_HANDLDER_H_