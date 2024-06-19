#include <iostream>

#include "Event.hpp"

void RehashEvent::Handler() {
    std::cout << "rehash event" << std::endl;
}

void DumpEvent::Handler() {
    std::cout << "dump event" << std::endl;
}

void ExitEvent::Handler() {
    std::cout << "exit event" << std::endl;
}