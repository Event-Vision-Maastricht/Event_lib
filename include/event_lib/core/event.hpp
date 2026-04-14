#ifndef EVENT_LIB_CORE_EVENT_HPP
#define EVENT_LIB_CORE_EVENT_HPP

#include <cstdint>
#include <iostream>
#include <vector>

namespace event_lib {

    /**
     * @struct Event
     * @brief Represents a single event
     *
     * Classic representation of a DVS event with only the basics.
     */
    struct Event {
        long long timestamp;  //in microseconds, might change to miliseconds
        int x;               // X coordinate
        int y;               // y coordinate
        bool polarity;       // true = on, false = off

        // Initialized as 0000
        Event() : timestamp(0), x(0), y(0), polarity(false) {}

        // Parameterized constructor
        Event(long long ts, int x_coord, int y_coord, bool pol)
            : timestamp(ts), x(x_coord), y(y_coord), polarity(pol) {}

        // Equality operator e1==e2 if ts, x, y and pol are equal, might be needed
        bool operator==(const Event& other) const {
            return timestamp == other.timestamp &&
                x == other.x &&
                y == other.y &&
                polarity == other.polarity;
        }

        // Inequality operator 
        bool operator!=(const Event& other) const {
            return !(*this == other);
        }
    };

    /**
     * @brief Create an event with specified parameters
     * @param ts Timestamp in microseconds
     * @param x X coordinate
     * @param y Y coordinate
     * @param polarity Event polarity (true = ON, false = OFF)
     * @return Created Event object
     */
    Event create_event(long long ts, int x, int y, bool polarity);

    /**
     * @brief Print event information to output stream
     * @param event The event to print
     */
    void print_event(const Event& event);

    /**
     * @brief Validate if event coordinates are within sensor bounds
     * @param event The event to validate
     * @param width Sensor width
     * @param height Sensor height
     * @return true if event is within bounds, false otherwise
     */
    bool is_valid_event(const Event& event, int width, int height);

    long long get_timestamp(Event e);
    std::vector<int> get_coordinates(Event e);
    bool get_polarity(Event e);
}

#endif