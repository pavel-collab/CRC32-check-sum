#include <syslog.h>

#include <iostream>
#include <vector>
#include <algorithm>

#include "Event.hpp"
#include "crc32.hpp"

void CrcInitializeEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    //TODO: delete on release
    std::cout << "initial event" << std::endl;

    std::vector<std::string> file_list;
    GetObjectList(this->path_to_dir_.c_str(), &file_list);
    // если в момент расчета затригериться событие проверки, это не нарушит целостности мапы, так как события складываются в очередь
    for (auto &file : file_list) {
        std::string path_to_file = this->path_to_dir_ + "/" + file;
        unsigned int crc_check_sum = ChecSum(path_to_file.c_str());
        (*crc_sums)[path_to_file] = crc_check_sum;
    }
}

void CheckSumEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    //TODO: delete on release
    std::cout << "check sum event" << std::endl;
    
    bool integrity_check = true;
    for (auto &file : *crc_sums) {
        unsigned int crc_sum = ChecSum(file.first.c_str());
        if (crc_sum != file.second) {
            openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
            syslog(LOG_INFO, "[err] integrity check: FAIL (file: %s -- expected 0x%08x, but got 0x%08x)\n", file.first.c_str(), file.second, crc_sum);
            closelog();
            (*crc_sums)[file.first] = crc_sum;
            integrity_check = false;
        }
    }

    if (integrity_check) {
        openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "integrity check: OK\n");
        closelog();
    }
}

void AddFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    //TODO: delete on release
    std::cout << "add file event" << std::endl;

    std::string path_to_file = this->path_to_dir_ + "/" + this->file_name_;
    unsigned int crc_check_sum = ChecSum(path_to_file.c_str());
    (*crc_sums)[path_to_file] = crc_check_sum;
}

void RmFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    //TODO: delete on release
    std::cout << "remove event" << std::endl;

    std::string path_to_file = this->path_to_dir_ + "/" + this->file_name_;
    crc_sums->erase(path_to_file);
}

void CheckFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    std::cout << "check file event" << std::endl;
    std::string path_to_file = this->path_to_dir_ + "/" + this->file_name_;
    unsigned int crc_sum = ChecSum(path_to_file.c_str());
    if (crc_sum != (*crc_sums)[path_to_file]) {
        openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "[err] integrity check: FAIL (file: %s -- expected 0x%08x, but got 0x%08x)\n", path_to_file.c_str(), (*crc_sums)[path_to_file], crc_sum);
        closelog();
        (*crc_sums)[path_to_file] = crc_sum;
    } else {
        openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "integrity check: OK\n");
        closelog();
    }
}

void ExitEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    std::cout << "exit event" << std::endl;
}