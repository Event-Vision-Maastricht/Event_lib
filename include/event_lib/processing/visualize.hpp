#pragma once

#include "../core/event_packet.hpp"
#include <vector>

namespace event_lib {
public:
    std::vector<int> make_histogram(const EventPacket& packet, bool colorOn=1);
    void timew_histogram(const EventPacket& packet, bool colorOn=1);
    void eventc_histogram(const EventPacket& packet, bool colorOn=1);
    void make_bi(const EventPacket& packet);
    void make_time_surface(const EventPacket& packet);



}