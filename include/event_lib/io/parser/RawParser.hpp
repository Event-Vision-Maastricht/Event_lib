#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <fstream>
#include "event_lib/core/event.hpp"
#include "event_lib/core/event_packet.hpp"
#include "event_lib/core/event_parser.hpp"
#include "event_lib/core/sensor_metadata.hpp"

namespace event_lib {

using RawFileHeader = SensorMetadata;
class RawParser final : public EventParser{
public:
	RawParser() = default;
    ~RawParser() override;

    void open(const std::string& path) override;
    bool has_more() const override;
    EventPacket read_packet(std::size_t max_events) override;
    bool reset() override;
    void close() override;
    int get_length();
    const RawFileHeader& header() const;

private:
    static void validate_raw_path(const std::string& path);
    RawFileHeader read_header();
    Event decode_event(const unsigned char* bytes) const;

    int length_ =0;
    std::string path_;
    std::ifstream file_;
    RawFileHeader header_;
    bool eof_reached_ = true;

};

}
