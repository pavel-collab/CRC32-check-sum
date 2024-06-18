#ifndef _LIB_H_
#define _LIB_H_

#include <vector>

#include "directory.hpp"

unsigned int Crc32(const unsigned char* buf, unsigned len);

int ChecSum(const char* filename);

std::vector<std::string> GetObjectList(const char* path_to_directory);

void CheckSumDerectory(Directory* dir);

#endif //_LIB_H_