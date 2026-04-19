#pragma once

#include "../../core/event_stream.hpp"

namespace event_lib {

class DVSCamera : public EventStream {
public:
    bool has_next() const override;
    Event next() override;
};

}