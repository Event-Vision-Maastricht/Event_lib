#pragma once

#include <string>
#include <vector>
#include "../../core/event.hpp"

namespace event_lib {

enum class FileType {Unknown, Raw, Dat, Hdf5};

class FileParser {
public:
    static FileType file_type(const std::string& path);
    static bool is_supported(const std::string& path);
    static std::vector<Event> load_events(const std::string& path);
};

}