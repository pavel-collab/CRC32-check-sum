#ifndef _DIRECTORY_H_
#define _DIRECTORY_H_

#include <vector>
#include <string>
#include <unordered_map>
#include <queue>

struct Directory {

    int flag;
    int periode;
    pthread_mutex_t mu;
    std::string path_to_directory;
    std::unordered_map<std::string, unsigned int> check_sum_container;
};

#endif //_DIRECTORY_H_