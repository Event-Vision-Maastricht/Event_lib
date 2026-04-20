#include "event_lib/io/dataset/DatEventParser.hpp"
#include <fstream>
#include <stdexcept>

namespace event_lib {
    std::vector<Event> DatEventParser::parse(const std::string& path) {
        std::vector<Event> events;
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open .dat file: " + path);
        }
        // TODO: Read binary Event structs (32 bytes each)
        // Assume Event is POD: timestamp (8), x(4), y(4), polarity(1) = 17 bytes? Wait, check Event size.
        // For now, stub
        file.close();
        return events;
    }
}