#include <syslog.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "DumpMessage.hpp"
#include "Event.hpp"
#include "crc32.hpp"
#include "syslogDump.hpp"

void CrcInitializeEvent::Handler(
    std::unordered_map<std::string, unsigned int> *crc_sums,
    MessageManager *message_manager) {
  std::vector<std::string> file_list;
  GetObjectList(path_to_dir_.c_str(), &file_list);
  // if during the crc32 calculating check sum signal will be caught, it will
  // not destroy map, because all of the event are put into queue, and access to
  // the queue is under the mutex

  for (auto &file : file_list) {
    // calculate and save crc32 for each file in target directory

    std::string path_to_file = path_to_dir_ + "/" + file;
    unsigned int crc_check_sum = CalculateCrc32(path_to_file.c_str());
    (*crc_sums)[path_to_file] = crc_check_sum;
  }
}

void CheckSumEvent::Handler(
    std::unordered_map<std::string, unsigned int> *crc_sums,
    MessageManager *message_manager) {
  bool integrity_check = true;

  // check crc32 for all of the files
  for (auto &file : *crc_sums) {
    unsigned int crc_sum = CalculateCrc32(file.first.c_str());

    if (crc_sum != file.second) {
      // write message into sysloh
      SYSLOG_DUMP(
          "[err] integrity check: FAIL (file: %s -- expected 0x%08x, but "
          "got 0x%08x)\n",
          file.first.c_str(), file.second, crc_sum);

      // generate message for json log
      DumpMessage *new_message =
          new MessageFail{file.first, file.second, crc_sum};
      message_manager->addMessage(new_message);

      // recalculate crc32 for this file!
      (*crc_sums)[file.first] = crc_sum;
      integrity_check = false;
    } else {
      DumpMessage *new_message =
          new MessageOK(file.first, file.second, crc_sum);
      message_manager->addMessage(new_message);
    }
  }

  if (integrity_check) {
    SYSLOG_DUMP("integrity check: OK\n");
  }
}

void AddFileEvent::Handler(
    std::unordered_map<std::string, unsigned int> *crc_sums,
    MessageManager *message_manager) {
  std::string path_to_file = path_to_dir_ + "/" + file_name_;
  unsigned int crc_check_sum = CalculateCrc32(path_to_file.c_str());
  (*crc_sums)[path_to_file] = crc_check_sum;

  DumpMessage *new_message = new MessageNew{path_to_file};
  message_manager->addMessage(new_message);
}

void RmFileEvent::Handler(
    std::unordered_map<std::string, unsigned int> *crc_sums,
    MessageManager *message_manager) {
  std::string path_to_file = path_to_dir_ + "/" + file_name_;
  crc_sums->erase(path_to_file);

  DumpMessage *new_message = new MessageAbsent{path_to_file};
  message_manager->addMessage(new_message);
}

void CheckFileEvent::Handler(
    std::unordered_map<std::string, unsigned int> *crc_sums,
    MessageManager *message_manager) {
  std::string path_to_file = path_to_dir_ + "/" + file_name_;
  unsigned int crc_sum = CalculateCrc32(path_to_file.c_str());

  if (crc_sum != (*crc_sums)[path_to_file]) {
    SYSLOG_DUMP("[err] integrity check: FAIL (file: %s -- expected 0x%08x, but "
                "got %#08x)\n",
                path_to_file.c_str(), (*crc_sums)[path_to_file], crc_sum);

    DumpMessage *new_message =
        new MessageFail{path_to_file, (*crc_sums)[path_to_file], crc_sum};
    message_manager->addMessage(new_message);

    (*crc_sums)[path_to_file] = crc_sum;
  } else {
    SYSLOG_DUMP("integrity check: OK\n");

    DumpMessage *new_message =
        new MessageOK(path_to_file, (*crc_sums)[path_to_file], crc_sum);
    message_manager->addMessage(new_message);
  }
}

void ExitEvent::Handler(std::unordered_map<std::string, unsigned int> *crc_sums,
                        MessageManager *message_manager) {
  SYSLOG_DUMP("[inf] daemon stop\n");
  message_manager->dumpJsonLog();
}