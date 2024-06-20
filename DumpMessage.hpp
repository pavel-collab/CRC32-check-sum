#ifndef _DUMP_MESSAGE_H_
#define _DUMP_MESSAGE_H_

#include <string>

#include "json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;

enum class MessageStatus {OK, FAIL, NEW, ABSENT};

/**
 * \brief Interface for the message about events.
 * 
 * This is the interface for the message that contain brief information about events are monitored by demon.
 * There are 4 different messages: check file crc32 -- ok, check file crc32 -- fail, new file creation, file removing.
 * Such object is able to be interpete in json format. So, using this objects is a way to save event history in json format.
 */
class DumpMessage {
protected:
    std::string path_to_file_;
    MessageStatus status_;
    std::string time_;

    /**
   * This method represent the crc32 check sum in sutable format 0x00000000 to write it in json.
   * @param crc32 -- unsigned int number (crc32 file check sum)
   */
    std::string FormatCRC32(unsigned int crc32);

    /**
   * This method returns string with current time. Time is also wrote into json log to analyse it more comfortable.
   */
    std::string GetCurrentTimeUTC();
public:
    DumpMessage(std::string path_to_file): path_to_file_(path_to_file) {};
    
    /**
   * This method generates json object and write the information about some event into this object.
   */
    virtual json DumpToJsonObj() = 0;
    virtual ~DumpMessage() {};
};

class MessageOK final: public DumpMessage {
private:
    unsigned int etalon_crc_;
    unsigned int result_crc_;
public:
    MessageOK(std::string path_to_file, unsigned int etalon_crc, unsigned int result_crc): DumpMessage(path_to_file), etalon_crc_(etalon_crc), result_crc_(result_crc) {
        this->status_ = MessageStatus::OK;
        time_ = GetCurrentTimeUTC();
    };

    json DumpToJsonObj() override;
};

class MessageFail final: public DumpMessage {
private:
    unsigned int etalon_crc_;
    unsigned int result_crc_;
public:
    MessageFail(std::string path_to_file, unsigned int etalon_crc, unsigned int result_crc): DumpMessage(path_to_file), etalon_crc_(etalon_crc), result_crc_(result_crc) {
        this->status_ = MessageStatus::FAIL;
        time_ = GetCurrentTimeUTC();
    };

    json DumpToJsonObj() override;
};

class MessageNew final: public DumpMessage {
public:
    MessageNew(std::string path_to_file): DumpMessage(path_to_file){
        this->status_ = MessageStatus::NEW;
        time_ = GetCurrentTimeUTC();
    };

    json DumpToJsonObj() override;
};

class MessageAbsent final: public DumpMessage {
public:
    MessageAbsent(std::string path_to_file): DumpMessage(path_to_file){
        this->status_ = MessageStatus::ABSENT;
        time_ = GetCurrentTimeUTC();
    };

    json DumpToJsonObj() override;
};

#endif