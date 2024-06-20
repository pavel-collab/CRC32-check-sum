#ifndef _LIB_H_
//TODO: _LIB_H_ -> _CRC_32_H
#define _LIB_H_

/**
 * Function calculates crc32 sum for given file.
 * @param filename -- name of the file
 */
int ChecSum(const char* filename);

/**
 * Function scan given directory and write names of files in this directory to the given vector.
 * @param path_to_directory -- path to the given directory
 * @param file_list -- pointer to vector
 */
void GetObjectList(const char* path_to_directory, std::vector<std::string>* file_list);

#endif //_LIB_H_