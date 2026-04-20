#pragma once
#include <string>
#include <vector>
#include "../../core/event.hpp"

namespace event_lib {
    class DatEventParser {
    public:
        // Reads binary Event stream (32 bytes per event)
        static std::vector<Event> parse(const std::string& path);
    };
}
