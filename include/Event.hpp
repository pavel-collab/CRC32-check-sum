#ifndef _EVENT_H_
#define _EVENT_H_

#include <iostream>
#include <string>
#include <unordered_map>

#include "DumpMessage.hpp"
#include "MessageManager.hpp"

enum class EventId {
  CrcInitialize,
  CheckSum,
  AddFile,
  RmFile,
  CheckFile,
  Exit
};

/**
 * \brief Interface class for the event in the system.
 *
 * This class describe the interface for the event, that Daemon have to monitor.
 * Object of Event incapsulate path to the directory that Daemon monitors.
 */
class Event {
protected:
  std::string path_to_dir_;

public:
  Event(std::string path_to_dir) : path_to_dir_(path_to_dir){};
  EventId eventId;

  /**
   * This is a virtual method that each type of Event has to have.
   * Method describe actions that have to be executen when the event happens.
   * @param crc_sums -- hash table of the files in the derictory with their
   * crc32 check sums.
   * @param message_manager -- pointer to message manager that save event
   * history
   */
  virtual void Handler(std::unordered_map<std::string, unsigned int> *crc_sums,
                       MessageManager *message_manager) = 0;
  virtual ~Event(){};
};

/**
 * \brief Initial event.
 *
 * This type of event generates when Daemon is started.
 * CrcInitializeEvent triggers Daemon to scan files in directory, calculate it's
 * crc check sums and save it.
 */
class CrcInitializeEvent final : public Event {
public:
  CrcInitializeEvent(std::string path_to_dir) : Event(path_to_dir) {
    eventId = EventId::CheckSum;
  };

  void Handler(std::unordered_map<std::string, unsigned int> *crc_sums,
               MessageManager *message_manager) override;
  ~CrcInitializeEvent(){};
};

/**
 * \brief Event that triggers Daemon to check crc32 sums for all of the files in
 * the directory.
 */
class CheckSumEvent final : public Event {
public:
  CheckSumEvent(std::string path_to_dir) : Event(path_to_dir) {
    eventId = EventId::CheckSum;
  };

  void Handler(std::unordered_map<std::string, unsigned int> *crc_sums,
               MessageManager *message_manager) override;
  ~CheckSumEvent(){};
};

/**
 * \brief Event that generated when new file is created in the directory.
 * Triggers Daemon to calculate and save crc32 sum for the new file.
 */
class AddFileEvent final : public Event {
  std::string file_name_;

public:
  AddFileEvent(std::string path_to_dir, std::string file_name)
      : Event(path_to_dir), file_name_(file_name) {
    eventId = EventId::AddFile;
  };

  void Handler(std::unordered_map<std::string, unsigned int> *crc_sums,
               MessageManager *message_manager) override;
  ~AddFileEvent(){};
};

/**
 * \brief Event that generated when some file is removed from the directory.
 * Triggers the Daemon to erase information about this file.
 */
class RmFileEvent final : public Event {
  std::string file_name_;

public:
  RmFileEvent(std::string path_to_dir, std::string file_name)
      : Event(path_to_dir), file_name_(file_name) {
    eventId = EventId::RmFile;
  };

  void Handler(std::unordered_map<std::string, unsigned int> *crc_sums,
               MessageManager *message_manager) override;
  ~RmFileEvent(){};
};

/**
 * \brief Event that generated when some file is modified. Triggers Daemon to
 * check crc32 check sum for this file.
 */
class CheckFileEvent final : public Event {
  std::string file_name_;

public:
  CheckFileEvent(std::string path_to_dir, std::string file_name)
      : Event(path_to_dir), file_name_(file_name) {
    eventId = EventId::CheckFile;
  };

  void Handler(std::unordered_map<std::string, unsigned int> *crc_sums,
               MessageManager *message_manager) override;
  ~CheckFileEvent(){};
};

/**
 * \brief Event that generated when system catch the signal to stop Daemon.
 * Trigges Daemon to finish his proces.
 */
class ExitEvent final : public Event {
public:
  ExitEvent(std::string path_to_dir) : Event(path_to_dir) {
    eventId = EventId::Exit;
  };

  void Handler(std::unordered_map<std::string, unsigned int> *crc_sums,
               MessageManager *message_manager) override;
  ~ExitEvent(){};
};

#endif