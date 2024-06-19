#ifndef _LIB_H_
#define _LIB_H_

#include <vector>

#include "directory.hpp"

unsigned int Crc32(const unsigned char* buf, unsigned len);

int ChecSum(const char* filename);

void GetObjectList(const char* path_to_directory, std::vector<std::string>* file_list);

#endif //_LIB_H_