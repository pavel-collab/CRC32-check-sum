#include <fstream>

#include "MessageManager.hpp"
#include "syslogDump.hpp"

void MessageManager::dumpJsonLog() {
  json general_json_obj(message_vector_);
  std::ofstream json_log_file(path_to_json_log_);
  if (json_log_file.is_open()) {
    json_log_file << general_json_obj.dump();
    json_log_file.close();
  } else {
    SYSLOG_DUMP("[err] unable to dump long info into json file\n");
  }
}

void MessageManager::addMessage(DumpMessage *message) {
  json new_json_message = message->dumpToJsonObj();
  message_vector_.push_back(new_json_message);
}