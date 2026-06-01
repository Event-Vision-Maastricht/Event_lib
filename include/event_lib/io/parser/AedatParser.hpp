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

using AedatFileHeader = SensorMetadata;
class AedatParser final : public EventParser{
public:
    AedatParser() = default;
    ~AedatParser() override;

    void open(const std::string& path) override;
    bool has_more() const override;
    EventPacket read_packet(std::size_t max_events) override;
    bool reset() override;
    void close() override;
    int get_length();
    const AedatFileHeader& header() const;

private:
    static void validate_aedat_path(const std::string& path);
    AedatFileHeader read_header();
    bool decode_event(const unsigned char* bytes, Event& out_event) const;

    int length_ =0;
    std::string path_;
    std::ifstream file_;
    AedatFileHeader header_;
    bool eof_reached_ = true;
};

}
