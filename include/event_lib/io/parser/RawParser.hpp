#pragma once

#include <string>
#include <vector>
#include "../../core/event.hpp"

namespace event_lib {

class RawParser {
public:
	static std::vector<Event> parse(const std::string& path);
};

}
