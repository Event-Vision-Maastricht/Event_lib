#ifndef EVENT_LIB_CORE_EVENT_PACKET_HPP
#define EVENT_LIB_CORE_EVENT_PACKET_HPP

#include <vector>
#include "event.hpp"

namespace event_lib {
    /**
     * @class EventPacket
     * @brief Container for managing a collection of events
     *
     * EventPacket stores and manages multiple events from any source
     * (camera, dataset, file, etc.). It provides utilities for adding,
     * removing, querying, and filtering events.
     */
    class EventPacket {
    public:

        //add an event to the package
        void add_event(const Event& e);

        //get all the events, read only
        const std::vector<Event>& get_events() const;

        //remove an event by index, idk if needed
        bool remove_event(size_t index);

        //returns the total number of events in the packet
        size_t size() const;

        //return number of events in a time window
        size_t count_in_range(long w_start, long w_end) const;

        //return events in a time window
        std::vector<Event> get_events_in_range(long w_start, long w_end) const;

        //clean up events
        void clear();

        //check if packet empty
        bool is_empty() const;

        // Equality operator p1==p2 if each event is the same in the same order.
        bool operator==(const EventPacket& other) const {
            return events == other.events;
        }

    private:
        std::vector<Event> events; //event container
        int current = 0;
    };
}

#endif