#pragma once

#include <cstddef>
#include <string>
#include "event_lib/core/event_packet.hpp"
#include "event_lib/core/sensor_metadata.hpp"

namespace event_lib {
    /**
     * @brief parser interface for abstracting parsers for different files. 
     */
    class EventParser {
    public:
        virtual ~EventParser() = default;

        virtual void open(const std::string& path) =0;
        virtual bool has_more() const = 0;
        virtual EventPacket read_packet(std::size_t max_events) = 0;
        virtual bool reset() =0;
        virtual void close() =0;
        // Return header information by const-reference. Must remain valid
        // for the lifetime of the parser instance.
        virtual const FileHeader& header() const = 0;    
    };
}
