#ifndef _DIRECTORY_H_
#define _DIRECTORY_H_

#include <vector>
#include <string>
#include <unordered_map>

struct Directory {
    std::string path_to_directory;
    std::unordered_map<std::string, unsigned int> check_sum_container;
};

struct ThreadArgs {
    int fd;
    int periode;
};

#endif //_DIRECTORY_H_