#ifndef _DEMON_H_
#define _DEMON_H_

#include <queue>
#include <string>
#include <unordered_map>

#include "json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;

#include "Event.hpp"
#include "DumpMessage.hpp"

class Demon;  // опережающее объявление

class DemonDestroyer final {
private:
    Demon* demon_instance;
public:    
    ~DemonDestroyer();
    void initialize(Demon* p);
};

/**
 * \brief Demon, that looks for the crc32 check sums of the files in some directory.
 * 
 * Demon save the crc32 check sums in the hash map for each file in the directory.
 * It incapsulates the queue of events and monitore it.
 * When next event generated it's put in this queue.
 * Demon give events from the queue one by one and do some actions by event method Handler.
 * In addition demon save the history of events to dump it in json format.
 */
class Demon final{
private:
    const char* path_to_json_log_ = "/tmp/log.json";
    static Demon* demon_instance;
    static DemonDestroyer destroyer;

    std::queue<Event*> event_queue_;
    pthread_mutex_t mutex;
    std::unordered_map<std::string, unsigned int> crc_sums;
    std::vector<json> message_vector;

    /**
   * Method dumps saved history of events in the json log file.
   */
    void DumpJsonLog();
protected: 
    Demon(std::string& path_to_dir): path_to_dir_(path_to_dir) {
        this->mutex = PTHREAD_MUTEX_INITIALIZER;
    };
    ~Demon() = default;
    friend class DemonDestroyer;
public:
    static Demon* getInstance(char* path_to_dir);  
    Demon(const Demon&) = delete;  

    /**
   * Method starts the loop. In this loop demon monitore the queue of events.
   * Loop is rolled until the ExitEvent will be generated.
   */
    void startMainLoop();

    /**
   * As external objects have not access to the queue of events, this method is a setter to put new event in the queue.
   * @param event -- pointer to the event object.
   */
    void addEvent(Event* event);

    std::string path_to_dir_;
};

#endif