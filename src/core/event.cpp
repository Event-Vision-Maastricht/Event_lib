#include "event_lib/core/event.hpp"

namespace event_lib {

    Event create_event(long long ts, int x, int y, bool polarity) {
        return Event(ts, x, y, polarity);
    }

    void print_event(const Event& event) {
        std::cout << "Event { "
                  << "timestamp: " << event.timestamp << ", "
                  << "x: " << event.x << ", "
                  << "y: " << event.y << ", "
                  << "polarity: " << (event.polarity ? "ON" : "OFF") << " }"
                  << std::endl;
    }

    bool is_valid_event(const Event& event, int width, int height) {
        return event.x >= 0 && event.x < width &&
               event.y >= 0 && event.y < height &&
               event.timestamp >= 0;
    }

}
