#pragma once

#include <memory>
#include <string>
#include "event_lib/core/event_parser.hpp"

namespace event_lib {

class EventParserFactory {
    public:
        static std::unique_ptr<EventParser> create_parser(const std::string& path);
    };
}
