#ifndef _MESSAGE_MANAGER_H_
#define _MESSAGE_MANAGER_H_

#include <vector>

#include "DumpMessage.hpp"

#include "json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * \brief This class saves the history of the events in json format.
 */
class MessageManager final {

  const char *path_to_json_log_ = "/tmp/log.json";
  std::vector<json> message_vector_;

public:
  /**
   * Method dumps saved history of events in the json log file.
   */
  void dumpJsonLog();

  void addMessage(DumpMessage *message);
};

#endif