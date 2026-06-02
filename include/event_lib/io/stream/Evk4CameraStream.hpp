#pragma once

#include "../../core/event_stream.hpp"
#include <cstddef>

namespace event_lib {

class DVSCamera : public EventStream {
public:
    bool has_next() const override;
    Event next() override;
    EventPacket next_packet(std::size_t max_events = 1024) override;
};

}