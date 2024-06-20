#include <unistd.h>
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

/**
 * Function calculates crc32 using data from given buffer
 * @param buf -- pointer to buffer with data
 * @param len -- number of bytes in buffer
 */
unsigned int Crc32(const unsigned char* buf, unsigned len) {
    unsigned int crc = 0xFFFFFFFF;
    while (len--)
        crc = (crc >> 8) ^ Crc32Table[(crc ^ *buf++) & 0xFF];
    return crc ^ 0xFFFFFFFF;
}

unsigned int CalculateCrc32(const char* filename) {
    // get the file size
    struct stat st;
    stat(filename, &st);
    long long file_size = (long long) st.st_size;

    unsigned char* buf = new unsigned char[max_len];

    // __O_NOATIME Do not update the file last access time (see man 2 open)
    int file = open(filename, O_RDONLY | __O_NOATIME);
    if (file < 0) {
        openlog("CRC32 daemon", LOG_CONS | LOG_PID, LOG_LOCAL0);
        syslog(LOG_INFO, "[err] unable to open file %s\n", filename);
        closelog();
        exit(EXIT_FAILURE);
    }

    unsigned int check_sum = 0;
    while(file_size > 0) {

        ssize_t read_symb_amount = read(file, buf, max_len);

        if (read_symb_amount < 0) {
            openlog("CRC32 daemon", LOG_CONS | LOG_PID, LOG_LOCAL0);
            syslog(LOG_INFO, "[err] unable to read file %s\n", filename);
            closelog();
            close(file);
            exit(EXIT_FAILURE);
        }

        file_size -= read_symb_amount;
        check_sum += Crc32(buf, read_symb_amount);
        memset(buf, 0, max_len);
    }
    close(file);
    delete[] buf;
    return check_sum;
}


int filter(const struct dirent *name) {
    return 1;
}

void GetObjectList(const char* path_to_directory, std::vector<std::string>* file_list) {
    struct dirent **namelist;
    int n = scandir(path_to_directory, &namelist, filter, alphasort);
    if (n == -1) {
        openlog("CRC32 daemon", LOG_CONS | LOG_PID, LOG_LOCAL0);
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