#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <fstream>
#include "event_lib/core/event.hpp"
#include "event_lib/core/event_packet.hpp"
#include "event_lib/core/event_parser.hpp"
//////////////     decode data

namespace event_lib {

struct DatFileHeader {


    int width = 0; // Horizontal size of image sensor array.
    int height = 0;// Vertical size of image sensor array.
    std::string date;/////     Recording Date, format: YYYY-MM-DD HH:MM:SS
    std::string time;
    std::string version;// Format version
    std::string event_type; //Type of event: CD/2d/ExtTrig
};
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

    DatFileHeader header() const;

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
