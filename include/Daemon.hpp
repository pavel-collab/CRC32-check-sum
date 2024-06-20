#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <queue>
#include <string>
#include <unordered_map>

#include "json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;

#include "Event.hpp"
#include "DumpMessage.hpp"

class Daemon;  // опережающее объявление

class DaemonDestroyer final {
private:
    Daemon* daemon_instance;
public:    
    ~DaemonDestroyer();

    void initialize(Daemon* p) {
        daemon_instance = p; 
    }
};

/**
 * \brief Daemon, that looks for the crc32 check sums of the files in the directory.
 * 
 * Daemon saves the crc32 check sums in the hash map for each file in the directory.
 * This incapsulates the queue of events and monitor it.
 * New emitted event is pushed into the queue.
 * Daemon gives events from the queue one by one and do some actions by event method Handler.
 * In addition, daemon saves the history of events to dump it to json format.
 */
class Daemon final {
private:
    const char* path_to_json_log_ = "/tmp/log.json";
    static Daemon* daemon_instance;
    static DaemonDestroyer destroyer;
    std::string path_to_dir_;

    std::queue<Event*> event_queue_;
    pthread_mutex_t mutex_;
    std::unordered_map<std::string, unsigned int> crc_sums_;
    std::vector<json> message_vector_;

    /**
   * Method dumps saved history of events in the json log file.
   */
    void dumpJsonLog();
protected: 
    Daemon(std::string& path_to_dir): path_to_dir_(path_to_dir), mutex_(PTHREAD_MUTEX_INITIALIZER) {};
    friend class DaemonDestroyer;
public:
    static Daemon* getInstance(char* path_to_dir);  
    Daemon(const Daemon&) = delete;  

    /**
   * Method starts the loop. In this loop Daemon monitore the queue of events.
   * loop is working until the ExitEvent is emitted.
   */
    void startMainLoop();

    /**
   * As external objects have not access to the queue of events, this method is a setter to put new event in the queue.
   * @param event -- pointer to the event object.
   */
    void addEvent(Event* event);

    std::string getTargetDirPath() {return path_to_dir_;};
};

#endif
