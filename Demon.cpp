#include "Demon.hpp"
#include "Event.hpp"

Demon* Demon::demon_instance = nullptr;
DemonDestroyer Demon::destroyer;

DemonDestroyer::~DemonDestroyer() {   
    delete demon_instance; 
}
void DemonDestroyer::initialize(Demon* p) {
    demon_instance = p; 
}
Demon* Demon::getInstance(char* path_to_dir) {
    std::string dir_path = std::string(path_to_dir);
    if(!demon_instance) {
        demon_instance = new Demon(dir_path);
        destroyer.initialize(demon_instance);     
    }
    return demon_instance;
}

void Demon::startMainLoop() {
    while(1) {
        if (!this->event_queue_.empty()) {
            Event* new_event = this->event_queue_.front();
            this->event_queue_.pop();

            new_event->Handler();
        }
    }
}