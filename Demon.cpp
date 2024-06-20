#include <syslog.h>

#include "Demon.hpp"
#include "Event.hpp"

#include <fstream>

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
                DumpJsonLog();
                new_event->Handler(&this->crc_sums, &this->message_vector);
                break;
            }

            new_event->Handler(&this->crc_sums, &this->message_vector);
        }
    }
}

void Demon::addEvent(Event* event) {
    pthread_mutex_lock(&this->mutex);
    this->event_queue_.push(event);
    pthread_mutex_unlock(&this->mutex);
}

void Demon::DumpJsonLog() {
    json general_json_obj(this->message_vector);
    std::ofstream json_log_file(path_to_json_log_);
    if (json_log_file.is_open()) {
        json_log_file << general_json_obj.dump();
        json_log_file.close();
    } else {
        openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "[err] unable to dump long info into json file\n");
        closelog();
    }
}