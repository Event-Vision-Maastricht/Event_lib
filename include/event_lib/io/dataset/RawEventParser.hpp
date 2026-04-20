#pragma once
#include <string>
#include <vector>
#include "../../core/event.hpp"

namespace event_lib {
    class RawEventParser {
    public:
        // Similar to .dat, flexible for custom layouts
        static std::vector<Event> parse(const std::string& path);
    };
}