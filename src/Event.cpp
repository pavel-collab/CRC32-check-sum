#include <syslog.h>

#include <iostream>
#include <vector>
#include <algorithm>

#include "Event.hpp"
#include "crc32.hpp"
#include "DumpMessage.hpp"

void CrcInitializeEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    std::vector<std::string> file_list;
    GetObjectList(path_to_dir_.c_str(), &file_list);
    // if during the crc32 calculating check sum signal will be caught, it will not destroy map, because all of the event are put into queue, and access to the queue is under the mutex
    
    for (auto &file : file_list) {
        // calculate and save crc32 for each file in target directory

        std::string path_to_file = path_to_dir_ + "/" + file;
        unsigned int crc_check_sum = CalculateCrc32(path_to_file.c_str());
        (*crc_sums)[path_to_file] = crc_check_sum;
    }
}

void CheckSumEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    bool integrity_check = true;

    // check crc32 for all of the files
    for (auto &file : *crc_sums) {
        unsigned int crc_sum = CalculateCrc32(file.first.c_str());

        if (crc_sum != file.second) {
            // write message into sysloh
            openlog("CRC32 daemon", LOG_CONS | LOG_PID, LOG_LOCAL0);
            syslog(LOG_INFO, "[err] integrity check: FAIL (file: %s -- expected 0x%08x, but got 0x%08x)\n", file.first.c_str(), file.second, crc_sum);
            closelog();

            // generate message for json log
            DumpMessage* new_message = new MessageFail{file.first, file.second, crc_sum};
            message_vector->push_back(new_message->dumpToJsonObj());

            // recalculate crc32 for this file!
            (*crc_sums)[file.first] = crc_sum;
            integrity_check = false;
        } else {
            DumpMessage* new_message = new MessageOK(file.first, file.second, crc_sum);
            message_vector->push_back(new_message->dumpToJsonObj());
        }
    }

    if (integrity_check) {
        openlog("CRC32 daemon", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "integrity check: OK\n");
        closelog();
    }
}

void AddFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    std::string path_to_file = path_to_dir_ + "/" + file_name_;
    unsigned int crc_check_sum = CalculateCrc32(path_to_file.c_str());
    (*crc_sums)[path_to_file] = crc_check_sum;

    DumpMessage* new_message = new MessageNew{path_to_file};
    message_vector->push_back(new_message->dumpToJsonObj());
}

void RmFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    std::string path_to_file = path_to_dir_ + "/" + file_name_;
    crc_sums->erase(path_to_file);

    DumpMessage* new_message = new MessageAbsent{path_to_file};
    message_vector->push_back(new_message->dumpToJsonObj());
}

void CheckFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    std::string path_to_file = path_to_dir_ + "/" + file_name_;
    unsigned int crc_sum = CalculateCrc32(path_to_file.c_str());
    
    if (crc_sum != (*crc_sums)[path_to_file]) {
        openlog("CRC32 daemon", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "[err] integrity check: FAIL (file: %s -- expected 0x%08x, but got 0x%08x)\n", path_to_file.c_str(), (*crc_sums)[path_to_file], crc_sum);
        closelog();
        
        DumpMessage* new_message = new MessageFail{path_to_file, (*crc_sums)[path_to_file], crc_sum};
        message_vector->push_back(new_message->dumpToJsonObj());

        (*crc_sums)[path_to_file] = crc_sum;
    } else {
        openlog("CRC32 daemon", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "integrity check: OK\n");
        closelog();

        DumpMessage* new_message = new MessageOK(path_to_file, (*crc_sums)[path_to_file], crc_sum);
        message_vector->push_back(new_message->dumpToJsonObj());
    }
}

void ExitEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums, std::vector<json>* message_vector) {
    openlog("CRC32 daemon", LOG_CONS | LOG_PID, LOG_LOCAL0);
    syslog(LOG_INFO, "[inf] daemon stop\n");
    closelog();
}