#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <fstream>
#include "event_lib/core/event.hpp"
#include "event_lib/core/event_packet.hpp"
#include "event_lib/core/event_parser.hpp"
#include "event_lib/core/sensor_metadata.hpp"
//////////////     decode data

namespace event_lib {

// Use SensorMetadata directly; DatFileHeader is kept as an alias for backward compatibility
using DatFileHeader = SensorMetadata;
//The event size is 8 for all those types.
class DatParser final : public EventParser{
public:
    DatParser() = default;
    ~DatParser() override;

    void open(const std::string& path) override;
    bool has_more() const override;
    EventPacket read_packet(std::size_t max_events) override;
    bool reset() override;
    void close() override;
    int get_length();
    const DatFileHeader& header() const;

private:
    static void validate_dat_path(const std::string& path);
    DatFileHeader read_header();
    Event decode_event(const unsigned char* bytes) const;

    int length_ =0;
    std::string path_;
    std::ifstream file_;
    DatFileHeader header_;
    bool eof_reached_ = true;
};

}
