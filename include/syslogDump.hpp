#ifndef _SYSLOG_DUMP_H_
#define _SYSLOG_DUMP_H_

#include <syslog.h>

#define SYSLOG_DUMP(...)                                                       \
  do {                                                                         \
    openlog("CRC32 daemon", LOG_CONS | LOG_PID, LOG_LOCAL0);                   \
    syslog(LOG_INFO, __VA_ARGS__);                                             \
    closelog();                                                                \
  } while (0)

#endif