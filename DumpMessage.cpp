#include <vector>

#include "DumpMessage.hpp"

json MessageOK::DumpToJsonObj() {
    json new_json_obj;
    new_json_obj["path"] = this->path_to_file_;
    new_json_obj["etalon_crc32"] = FormatCRC32(this->etalon_crc_);
    new_json_obj["result_crc32"] = FormatCRC32(this->result_crc_);
    new_json_obj["status"] = "OK";

    return new_json_obj;
}

json MessageFail::DumpToJsonObj() {
    json new_json_obj;
    new_json_obj["path"] = this->path_to_file_;
    new_json_obj["etalon_crc32"] = FormatCRC32(this->etalon_crc_);
    new_json_obj["result_crc32"] = FormatCRC32(this->result_crc_);
    new_json_obj["status"] = "FAIL";
    new_json_obj["time"] = this->time_;

    return new_json_obj;
}

json MessageNew::DumpToJsonObj() {
    json new_json_obj;
    new_json_obj["path"] = this->path_to_file_;
    new_json_obj["status"] = "NEW";
    new_json_obj["time"] = this->time_;

    return new_json_obj;
}

json MessageAbsent::DumpToJsonObj() {
    json new_json_obj;
    new_json_obj["path"] = this->path_to_file_;
    new_json_obj["status"] = "ABSENT";
    new_json_obj["time"] = this->time_;

    return new_json_obj;
}

std::string DumpMessage::FormatCRC32(unsigned int crc32) {
    char buf[11];
    sprintf(buf, "0x%08x", crc32);
    return std::string(buf);
}

std::string DumpMessage::GetCurrentTimeUTC() {
    time_t now = time(0);
    tm* gmtm = gmtime(&now);
    std::string dt = std::string(asctime(gmtm));
    return dt;
}