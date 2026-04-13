#pragma once

#include "../core/event_packet.hpp"
#include <vector>

namespace event_lib {

std::vector<int> make_histogram(const EventPacket& packet);

}