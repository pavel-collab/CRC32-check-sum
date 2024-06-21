#include <syslog.h>

#include <stdexcept>

#include "Daemon.hpp"
#include "Event.hpp"
#include "syslogDump.hpp"

Daemon *Daemon::daemon_instance = nullptr;
DaemonDestroyer Daemon::destroyer;

Daemon *Daemon::getInstance(char *path_to_dir) {
  std::string dir_path = std::string(path_to_dir);
  if (!daemon_instance) {
    daemon_instance = new Daemon(dir_path);
    destroyer.initialize(daemon_instance);
  }
  return daemon_instance;
}

void Daemon::startMainLoop() {

  // firstly generate initial event, it trigges Daemon to save crc32 check sums
  // for all of files in target dir
  Event *start_event = new CrcInitializeEvent{path_to_dir_};
  addEvent(start_event);

  while (1) {
    try {
      if (!event_queue_.empty()) {
        // as main proces and inotify thread are able to out events into the
        // queue, all actions with queue must executed under mutex
        pthread_mutex_lock(&mutex_);
        // give the next event from the queue
        Event *new_event = event_queue_.front();
        event_queue_.pop();
        pthread_mutex_unlock(&mutex_);

        if (new_event->eventId == EventId::Exit) {
          new_event->Handler(&crc_sums_, &message_manager_);
          break;
        }

        // execute logic, triggered by this event
        new_event->Handler(&crc_sums_, &message_manager_);
      }
    } catch (const std::runtime_error &error) {
      message_manager_.dumpJsonLog();
      break;
    }
  }
}

void Daemon::addEvent(Event *event) {
  pthread_mutex_lock(&mutex_);
  event_queue_.push(event);
  pthread_mutex_unlock(&mutex_);
}

DaemonDestroyer::~DaemonDestroyer() { delete daemon_instance; }