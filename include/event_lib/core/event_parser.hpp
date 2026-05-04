#pragma once

#include <cstddef>
#include <string>
#include "event_lib/core/event_packet.hpp"

namespace event_lib {
    // Simple POD header shared across parsers. Derived parser-specific
    // headers may extend this (e.g., DatFileHeader).
    struct FileHeader {
        int width = 0;  // Horizontal size of image sensor array.
        int height = 0; // Vertical size of image sensor array.
        std::string date; // Recording Date, format: YYYY-MM-DD HH:MM:SS
        std::string time;
        std::string version; // Format version
        std::string event_type; // Type of event: CD/2d/ExtTrig
        virtual ~FileHeader() = default;
    };
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
        virtual const FileHeader& header() const = 0;    };
}
