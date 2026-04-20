#pragma once

#include <string>
#include <vector>
#include "../../core/event.hpp"

namespace event_lib {

struct DatFileHeader {
    int width = 0;
    int height = 0;
    std::string date;
    std::string time;
    std::string version;
    std::string event_type;
};

class DatParser {
public:
    static std::vector<Event> parse(const std::string& path);
    static DatFileHeader read_header(const std::string& path);

private:
    static void validate_dat_path(const std::string& path);
};

}
