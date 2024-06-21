#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <queue>
#include <string>
#include <unordered_map>

#include "DumpMessage.hpp"
#include "Event.hpp"
#include "MessageManager.hpp"

class Daemon; // опережающее объявление

class DaemonDestroyer final {
private:
  Daemon *daemon_instance;

public:
  ~DaemonDestroyer();

  void initialize(Daemon *p) { daemon_instance = p; }
};

/**
 * \brief Daemon, that looks for the crc32 check sums of the files in the
 * directory.
 *
 * Daemon saves the crc32 check sums in the hash map for each file in the
 * directory. This incapsulates the queue of events and monitor it. New emitted
 * event is pushed into the queue. Daemon gives events from the queue one by one
 * and do some actions by event method Handler. In addition, daemon saves the
 * history of events to dump it to json format.
 */
class Daemon final {
private:
  static Daemon *daemon_instance;
  static DaemonDestroyer destroyer;
  MessageManager message_manager_;

  std::string path_to_dir_;

  std::queue<Event *> event_queue_;
  pthread_mutex_t mutex_;
  std::unordered_map<std::string, unsigned int> crc_sums_;

protected:
  Daemon(std::string &path_to_dir)
      : path_to_dir_(path_to_dir), mutex_(PTHREAD_MUTEX_INITIALIZER){};
  friend class DaemonDestroyer;

public:
  static Daemon *getInstance(char *path_to_dir);
  Daemon(const Daemon &) = delete;

  /**
   * Method starts the loop. In this loop Daemon monitore the queue of events.
   * loop is working until the ExitEvent is emitted.
   */
  void startMainLoop();

  /**
   * As external objects have not access to the queue of events, this method is
   * a setter to put new event in the queue.
   * @param event -- pointer to the event object.
   */
  void addEvent(Event *event);

  std::string getTargetDirPath() { return path_to_dir_; };
};

#endif
