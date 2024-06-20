// #include <stdlib.h>
// #include <assert.h>
// #include <sys/types.h>
// #include <sys/stat.h>
#include <unistd.h>
// #include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <syslog.h>

#include <vector>
#include <string>
#include <filesystem>

#include "table.hpp"

unsigned int Crc32(const unsigned char* buf, unsigned len) {
    unsigned int crc = 0xFFFFFFFF;
    while (len--)
        crc = (crc >> 8) ^ Crc32Table[(crc ^ *buf++) & 0xFF];
    return crc ^ 0xFFFFFFFF;
}

int ChecSum(const char* filename) {
    struct stat st;
    stat(filename, &st);
    long long file_size = (long long) st.st_size;

    unsigned char* buf = (unsigned char*) calloc(max_len, sizeof(char));

    //? что буедет, если во время подсчета контрольной суммы файл попробуют изменить?
    // флаг __O_NOATIME используется, чтобы при открытии файла время доступа к нему не менялось
    int file = open(filename, O_RDONLY | __O_NOATIME);
    if (file < 0) {
        //TODO: check the function return code
        openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "[err] unable to open file %s\n", filename);
        closelog();
        return -1;
    }

    unsigned int check_sum = 0;
    while(file_size > 0) {

        ssize_t read_symb_amount = read(file, buf, max_len);

        if (read_symb_amount < 0) {
            //TODO: check the function return code
            openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
            syslog(LOG_INFO, "[err] unable to read file %s\n", filename);
            closelog();
            close(file);
            return -1;
        }

        file_size -= read_symb_amount;
        check_sum += Crc32(buf, read_symb_amount);
        memset(buf, 0, max_len);
    }
    close(file);
    free(buf);
    return check_sum;
}


int filter(const struct dirent *name) {
    return 1;
}

void GetObjectList(const char* path_to_directory, std::vector<std::string>* file_list) {
    struct dirent **namelist;
    int n = scandir(path_to_directory, &namelist, filter, alphasort);
    if (n == -1) {
        //TODO: change the exit to the return code and check this code
        openlog("CRC32 DEMON", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "[err] unable to scan directory %s\n", path_to_directory);
        closelog();
        exit(EXIT_FAILURE);
    }

    while (n--) {
        switch(namelist[n]->d_type) {
        case DT_REG:
            file_list->emplace_back(namelist[n]->d_name);
            break;
        default:
            continue;
        }
    }
}