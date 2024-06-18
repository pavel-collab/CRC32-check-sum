#ifndef _EVENT_HANDLDER_H_
#define _EVENT_HANDLDER_H_

#include "directory.hpp"

void handle_events(int fd, Directory* dir);

void* event_main_loop(void* arg);

#endif //_EVENT_HANDLDER_H_