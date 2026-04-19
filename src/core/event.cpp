#include "event_lib/core/event.hpp"
#include <vector>

namespace event_lib {
//ts in ms
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

    long long get_timestamp(const Event& e) {
        return e.timestamp;
    }

    std::vector<int> get_coordinates(const Event& e) {
        return {e.x, e.y};
    }

    bool get_polarity(const Event& e) {
        return e.polarity;
    }

}
