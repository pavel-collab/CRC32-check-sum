#include <vector>

#include "DumpMessage.hpp"

#define BUF_SIZE 11

json MessageOK::dumpToJsonObj() {
    json new_json_obj;
    new_json_obj["path"] = path_to_file_;
    new_json_obj["etalon_crc32"] = formatCRC32(etalon_crc_);
    new_json_obj["result_crc32"] = formatCRC32(result_crc_);
    new_json_obj["status"] = "OK";

    return new_json_obj;
}

json MessageFail::dumpToJsonObj() {
    json new_json_obj;
    new_json_obj["path"] = path_to_file_;
    new_json_obj["etalon_crc32"] = formatCRC32(etalon_crc_);
    new_json_obj["result_crc32"] = formatCRC32(result_crc_);
    new_json_obj["status"] = "FAIL";
    new_json_obj["time"] = time_;

    return new_json_obj;
}

json MessageNew::dumpToJsonObj() {
    json new_json_obj;
    new_json_obj["path"] = path_to_file_;
    new_json_obj["status"] = "NEW";
    new_json_obj["time"] = time_;

    return new_json_obj;
}

json MessageAbsent::dumpToJsonObj() {
    json new_json_obj;
    new_json_obj["path"] = path_to_file_;
    new_json_obj["status"] = "ABSENT";
    new_json_obj["time"] = time_;

    return new_json_obj;
}

std::string DumpMessage::formatCRC32(unsigned int crc32) {
    char buf[BUF_SIZE];
    sprintf(buf, "%#08x", crc32);
    return std::string(buf);
}

std::string DumpMessage::getCurrentTimeUTC() {
    time_t now = time(0);
    tm* gmtm = gmtime(&now);
    std::string dt = std::string(asctime(gmtm));
    return dt;
}