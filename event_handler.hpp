#ifndef _EVENT_HANDLDER_H_
#define _EVENT_HANDLDER_H_

#include "directory.hpp"

void handle_events(int fd);

void* threadInotifyRun(void* arg);

void* threadDemonRun(void* arg);

#endif //_EVENT_HANDLDER_H_