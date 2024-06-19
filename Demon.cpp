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
    
    Event* start_event = new CrcInitializeEvent{this->path_to_dir_};
    this->addEvent(start_event);

    while(1) {
        if (!this->event_queue_.empty()) {
            pthread_mutex_lock(&this->mutex);
            Event* new_event = this->event_queue_.front();
            this->event_queue_.pop();
            pthread_mutex_unlock(&this->mutex);

            if (new_event->eventId == EventId::Exit) {
                new_event->Handler(&this->crc_sums);
                break;
            }

            new_event->Handler(&this->crc_sums);
        }
    }
}

void Demon::addEvent(Event* event) {
    pthread_mutex_lock(&this->mutex);
    this->event_queue_.push(event);
    pthread_mutex_unlock(&this->mutex);
}