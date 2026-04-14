#include "event_lib/core/event_packet.hpp"
#include <algorithm>

namespace event_lib {

    void EventPacket::add_event(const Event& e) {
        events.push_back(e);
    }

    const std::vector<Event>& EventPacket::get_events() const {
        return events;
    }

    //TODO: error handling, maybe a fail message
    bool EventPacket::remove_event(size_t idx) {
        if (idx >= events.size()) {
            return false;
        }
        events.erase(events.begin() + idx);
        return true;
    }

    size_t EventPacket::size() const {
        return events.size();
    }

    size_t EventPacket::count_in_range(long long w_start, long long w_end) const {
        return std::count_if(
            events.begin(),
            events.end(),
            [w_start, w_end](const Event& e) {
                return e.timestamp >= w_start && e.timestamp <= w_end;
            }
        );
    }

    std::vector<Event> EventPacket::get_events_in_range(long long w_start, long long w_end) const {
        std::vector<Event> result;
        for (const auto& e : events) {
            if (e.timestamp >= w_start && e.timestamp <= w_end) {
                result.push_back(e);
            }
        }
        return result;
    }

    void EventPacket::clear() {
        events.clear();
    }

    bool EventPacket::is_empty() const {
        return events.empty();
    }

}
