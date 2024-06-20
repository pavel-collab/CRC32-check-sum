#ifndef _EVENT_HANDLDER_H_
#define _EVENT_HANDLDER_H_

/**
 * This function is ran in thread that monitore events in target directory by
 * inotify.
 */
void *threadInotifyRun(void *arg);

/**
 * This function is ran in thread where we start the main Daemon loop
 */
void *threadDaemonRun(void *arg);

#endif //_EVENT_HANDLDER_H_