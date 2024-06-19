#ifndef _DEMON_H_
#define _DEMON_H_

#include <queue>
#include <string>

#include "Event.hpp"

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
    static Demon* demon_instance;
    static DemonDestroyer destroyer;

    std::queue<Event*> event_queue_;
    std::string path_to_dir_;
protected: 
    Demon(std::string& path_to_dir): path_to_dir_(path_to_dir) {};
    ~Demon() = default;
    friend class DemonDestroyer;
public:
    static Demon* getInstance(char* path_to_dir);  
    Demon(const Demon&) = delete;  

    void startMainLoop();
};

#endif