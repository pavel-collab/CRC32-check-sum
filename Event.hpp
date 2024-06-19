#ifndef _EVENT_H_
#define _EVENT_H_

#include <iostream>

enum class EventId {Rehash, Dump, Exit};

class Event {
public:
    EventId eventId;
    virtual void Handler() = 0;
    virtual ~Event() {};
};

class RehashEvent final: public Event {
public:
    RehashEvent() {
        eventId = EventId::Rehash;
    };

    void Handler() override;
    ~RehashEvent() {};
};

class DumpEvent final: public Event {
public:
    DumpEvent() {
        eventId = EventId::Dump;
    };

    void Handler() override;
    ~DumpEvent() {};
};

class ExitEvent final: public Event {
public:
    ExitEvent() {
        eventId = EventId::Exit;
    };

    void Handler() override;
    ~ExitEvent() {};
};

#endif