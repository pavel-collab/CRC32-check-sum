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

class Demon final{
private:
    const char* path_to_json_log_ = "/tmp/log.json";
    static Demon* demon_instance;
    static DemonDestroyer destroyer;

    std::queue<Event*> event_queue_;
    pthread_mutex_t mutex;
    std::unordered_map<std::string, unsigned int> crc_sums;
    std::vector<json> message_vector;

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

    void startMainLoop();
    void addEvent(Event* event);

    std::string path_to_dir_;
};

#endif