#ifndef _LIB_H_
#define _LIB_H_

int ChecSum(const char* filename);

void GetObjectList(const char* path_to_directory, std::vector<std::string>* file_list);

#endif //_LIB_H_