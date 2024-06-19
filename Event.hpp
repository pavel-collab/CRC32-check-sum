#ifndef _EVENT_H_
#define _EVENT_H_

#include <iostream>
#include <unordered_map>
#include <string>

enum class EventId {CrcInitialize, CheckSum, AddFile, RmFile, CheckFile, Exit};

class Event {
protected:
    std::string path_to_dir_;
public:
    Event(std::string path_to_dir): path_to_dir_(path_to_dir) {};
    EventId eventId;
    virtual void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) = 0;
    virtual ~Event() {};
};

class CrcInitializeEvent final: public Event {
public:
    CrcInitializeEvent(std::string path_to_dir): Event(path_to_dir) {
        eventId = EventId::CheckSum;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~CrcInitializeEvent() {};
};


class CheckSumEvent final: public Event {
public:
    CheckSumEvent(std::string path_to_dir): Event(path_to_dir) {
        eventId = EventId::CheckSum;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~CheckSumEvent() {};
};

class AddFileEvent final: public Event {
    std::string file_name_;
public:
    AddFileEvent(std::string path_to_dir, std::string file_name): Event(path_to_dir), file_name_(file_name) {
        eventId = EventId::AddFile;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~AddFileEvent() {};
};

class RmFileEvent final: public Event {
    std::string file_name_;
public:
    RmFileEvent(std::string path_to_dir, std::string file_name): Event(path_to_dir), file_name_(file_name) {
        eventId = EventId::RmFile;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~RmFileEvent() {};
};

class CheckFileEvent final: public Event {
    std::string file_name_;
public:
    CheckFileEvent(std::string path_to_dir, std::string file_name): Event(path_to_dir), file_name_(file_name) {
        eventId = EventId::CheckFile;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~CheckFileEvent() {};
};

class ExitEvent final: public Event {
public:
    ExitEvent(std::string path_to_dir): Event(path_to_dir) {
        eventId = EventId::Exit;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~ExitEvent() {};
};

#endif