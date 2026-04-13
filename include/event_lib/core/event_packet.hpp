#pragma once

#include <vector>
#include "event.hpp"

namespace event_lib{

    class EventPacket{
        public:
            void add_event(const Event& e);
            const std:: vector<Event>&get_events() const;

        private:
            std:: vector<Event> events;
    };
}