#include "event_lib/io/dataset/RawEventParser.hpp"
#include <fstream>
#include <stdexcept>

namespace event_lib {
    std::vector<Event> RawEventParser::parse(const std::string& path) {
        std::vector<Event> events;
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open .raw file: " + path);
        }
        // TODO: Implement raw parsing, flexible for custom layouts
        // For now, treat as .dat
        file.close();
        return events;
    }
}


