#ifndef EVENT_LIB_CORE_EVENT_STREAM_HPP
#define EVENT_LIB_CORE_EVENT_STREAM_HPP

#include "event.hpp"
#include <cstdint>

namespace event_lib {
    /**
     * Interface for sequentially reading events from different sources
     * (camera, dataset files, network, etc.).
     * Implementations inherit from this class, override is possible to provide specific reading logic
     * depending on if the data is getting fetched from camera or read from a dataset.
     *
     * EventStream* stream = new DatasetEventStream("events.dat");
     * while (stream->has_next()) {
     *     Event e = stream->next();
     *     // Process event
     * }
     * stream->close();
     * delete stream;
     */
    class EventStream {
    public:
        
        //Virtual destructor for proper cleanup in derived classes
        virtual ~EventStream() = default;

        // Check if there are more events available to read
        virtual bool has_next() const = 0;

        // Get the next event from the stream
        virtual Event next() = 0;

        // Reset the stream to the beginning
        virtual bool reset() {return false;} //default false bcs not reset

        // Close and cleanup the stream
        //TODO: could use to release the file and stuff, default nothing
        virtual void close() {}

        
        //TODO: Get the total number of events in this stream (check if its known in evk4)
        virtual long long get_event_count() const {
            return -1;  // Default: unknown, derived classes can override
        }
    };
}
#endif