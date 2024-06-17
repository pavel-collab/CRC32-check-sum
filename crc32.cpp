#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

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

void GetFileInfo(const char* filename,  struct stat *sb) {
    if (lstat(filename, sb) == -1) {
        perror("lstat");
        return;
    }

    // спомощью stat проверяем тип файла
    if ((sb->st_mode & S_IFMT) != S_IFREG) {
        puts("[err] Sorry, I can't proces this type of file this moment(");
        return;
    }
}

int ChecSum(const char* filename) {
    struct stat sb;
    GetFileInfo(filename, &sb);

    // аллоцирем буфер для чтения
    unsigned char* buf = (unsigned char*) calloc(max_len, sizeof(char));
    assert(buf != NULL);

    long long file_size = (long long) sb.st_size;

    //? что буедет, если во время подсчета контрольной суммы файл попробуют изменить?
    // флаг __O_NOATIME используется, чтобы при открытии файла время доступа к нему не менялось
    int file = open(filename, O_RDONLY | __O_NOATIME);
    if (file < 0) {
        perror("Failed for open copy file for writing");
        return -1;
    }

    unsigned int check_sum = 0;
    while(file_size > 0) {

        ssize_t read_symb_amount = read(file, buf, max_len);

        if (read_symb_amount < 0) {
            perror("Failed read from the file");
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

std::vector<std::string> GetObjectList(const char* path_to_directory) {
    struct dirent **namelist;
    int n = scandir(path_to_directory, &namelist, filter, alphasort);
    if (n == -1) {
        perror("scandir");
        exit(EXIT_FAILURE);
    }

    std::vector<std::string> file_list;
    while (n--) {
        switch(namelist[n]->d_type) {
        case DT_REG:
            file_list.emplace_back(namelist[n]->d_name);
            break;
        default:
            continue;
        }
    }

    return file_list;
}