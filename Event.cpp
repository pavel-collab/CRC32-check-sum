#include <syslog.h>

#include <iostream>
#include <vector>
#include <algorithm>

#include "Event.hpp"
#include "crc32.hpp"
#include "DumpMessage.hpp"

void CrcInitializeEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    std::vector<std::string> file_list;
    GetObjectList(this->path_to_dir_.c_str(), &file_list);
    // если в момент расчета затригериться событие проверки, это не нарушит целостности мапы, так как события складываются в очередь
    for (auto &file : file_list) {
        std::string path_to_file = this->path_to_dir_ + "/" + file;
        unsigned int crc_check_sum = ChecSum(path_to_file.c_str());
        (*crc_sums)[path_to_file] = crc_check_sum;
    }
}

void CheckSumEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    bool integrity_check = true;
    for (auto &file : *crc_sums) {
        unsigned int crc_sum = ChecSum(file.first.c_str());
        if (crc_sum != file.second) {
            openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
            syslog(LOG_INFO, "[err] integrity check: FAIL (file: %s -- expected 0x%08x, but got 0x%08x)\n", file.first.c_str(), file.second, crc_sum);
            closelog();
            DumpMessage* new_message = new MessageFail{file.first, file.second, crc_sum};
            message_vector->push_back(new_message->DumpToJsonObj());

            (*crc_sums)[file.first] = crc_sum;
            integrity_check = false;
        } else {
            DumpMessage* new_message = new MessageOK(file.first, file.second, crc_sum);
            message_vector->push_back(new_message->DumpToJsonObj());
        }
    }

    if (integrity_check) {
        openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "integrity check: OK\n");
        closelog();
    }
}

void AddFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    std::string path_to_file = this->path_to_dir_ + "/" + this->file_name_;
    unsigned int crc_check_sum = ChecSum(path_to_file.c_str());
    (*crc_sums)[path_to_file] = crc_check_sum;

    DumpMessage* new_message = new MessageNew{path_to_file};
    message_vector->push_back(new_message->DumpToJsonObj());
}

void RmFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    std::string path_to_file = this->path_to_dir_ + "/" + this->file_name_;
    crc_sums->erase(path_to_file);

    DumpMessage* new_message = new MessageAbsent{path_to_file};
    message_vector->push_back(new_message->DumpToJsonObj());
}

void CheckFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    std::string path_to_file = this->path_to_dir_ + "/" + this->file_name_;
    unsigned int crc_sum = ChecSum(path_to_file.c_str());
    if (crc_sum != (*crc_sums)[path_to_file]) {
        openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "[err] integrity check: FAIL (file: %s -- expected 0x%08x, but got 0x%08x)\n", path_to_file.c_str(), (*crc_sums)[path_to_file], crc_sum);
        closelog();
        
        DumpMessage* new_message = new MessageFail{path_to_file, (*crc_sums)[path_to_file], crc_sum};
        message_vector->push_back(new_message->DumpToJsonObj());

        (*crc_sums)[path_to_file] = crc_sum;
    } else {
        openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "integrity check: OK\n");
        closelog();

        DumpMessage* new_message = new MessageOK(path_to_file, (*crc_sums)[path_to_file], crc_sum);
        message_vector->push_back(new_message->DumpToJsonObj());
    }
}

void ExitEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
    syslog(LOG_INFO, "[inf] demon stop\n");
    closelog();
}