#pragma once
#include <string>
#include <vector>
#include "../../core/event.hpp"

namespace event_lib {
    class HdfEventParser {
    public:
        // TODO: Add HDF5 dependencies
        static std::vector<Event> parse(const std::string& path);
    };
}