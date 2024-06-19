#ifndef _EVENT_H_
#define _EVENT_H_

#include <iostream>
#include <unordered_map>
#include <string>

enum class EventId {CrcInitialize, CheckSum, AddFile, RmFile, CheckFile, Exit};

class Event {
public:
    EventId eventId;
    virtual void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) = 0;
    virtual ~Event() {};
};

class CrcInitializeEvent final: public Event {
public:
    CrcInitializeEvent() {
        eventId = EventId::CheckSum;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~CrcInitializeEvent() {};
};


class CheckSumEvent final: public Event {
public:
    CheckSumEvent() {
        eventId = EventId::CheckSum;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~CheckSumEvent() {};
};

class AddFileEvent final: public Event {
    std::string file_path_;
public:
    AddFileEvent(std::string file_path): file_path_(file_path) {
        eventId = EventId::AddFile;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~AddFileEvent() {};
};

class RmFileEvent final: public Event {
    std::string file_path_;
public:
    RmFileEvent(std::string file_path): file_path_(file_path) {
        eventId = EventId::RmFile;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~RmFileEvent() {};
};

class CheckFileEvent final: public Event {
    std::string file_path_;
public:
    CheckFileEvent(std::string file_path): file_path_(file_path) {
        eventId = EventId::CheckFile;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~CheckFileEvent() {};
};

class ExitEvent final: public Event {
public:
    ExitEvent() {
        eventId = EventId::Exit;
    };

    void Handler(std::unordered_map<std::string, unsigned int>* crc_sums) override;
    ~ExitEvent() {};
};

#endif