#include <iostream>

#include "Event.hpp"

void CrcInitializeEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    std::cout << "initial event" << std::endl;
}

void CheckSumEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    std::cout << "check sum event" << std::endl;
}

void AddFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    std::cout << "add file event" << std::endl;
}

void RmFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    std::cout << "remove event" << std::endl;
}

void CheckFileEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    std::cout << "check file event" << std::endl;
}

void ExitEvent::Handler(std::unordered_map<std::string, unsigned int>* crc_sums) {
    std::cout << "exit event" << std::endl;
}