#include "event_lib/io/parser/DatParser.hpp"
#include <array>
#include <cstring>
#include <filesystem>
#include <stdexcept>

namespace event_lib {

DatParser::~DatParser() {
    close();
}

void DatParser::open(const std::string& path) {
    validate_dat_path(path);

    close();
    path_ = path;
    file_.open(path_, std::ios::binary);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to open .dat file: " + path_);
    }

    header_ = read_header();
    eof_reached_ = false;
}

bool DatParser::has_more() const {
    return file_.is_open() && !eof_reached_;
}

EventPacket DatParser::read_packet(std::size_t max_events) {
    if (!file_.is_open()) {
        throw std::runtime_error("DatParser::read_packet called before open().");
    }
    if (max_events == 0) {
        throw std::runtime_error("max_events must be greater than zero.");
    }

    EventPacket packet;
    std::array<unsigned char, sizeof(long long) + sizeof(int) + sizeof(int) + sizeof(bool)> raw_event{};

    for (std::size_t i = 0; i < max_events; ++i) {
        file_.read(reinterpret_cast<char*>(raw_event.data()), static_cast<std::streamsize>(raw_event.size()));
        if (file_.gcount() != static_cast<std::streamsize>(raw_event.size())) {
            eof_reached_ = true;
            break;
        }

        packet.add_event(decode_event(raw_event.data()));
    }

    return packet;
}

bool DatParser::reset() {
    if (!file_.is_open()) return false;
    
    file_.clear();
    file_.seekg(0, std::ios::beg);
    header_ = read_header();
    eof_reached_ = false;
    return true;
}

void DatParser::close() {
    if (file_.is_open()) {
        file_.close();
    }
    eof_reached_ = true;
}

DatFileHeader DatParser::header() const {
    return header_;
}

void DatParser::validate_dat_path(const std::string& path) {
    const std::filesystem::path p(path);
    if (p.extension() != ".dat") {
        throw std::runtime_error("DatParser accepts only .dat files: " + path);
    }
}

//TODO
DatFileHeader DatParser::read_header() {
    DatFileHeader header;
    return header;
}


//TODO
Event DatParser::decode_event(const unsigned char* bytes) const {
    Event event;
    // std::size_t offset = 0;
    // std::memcpy(&event.timestamp, bytes + offset, sizeof(event.timestamp));
    // offset += sizeof(event.timestamp);
    // std::memcpy(&event.x, bytes + offset, sizeof(event.x));
    // offset += sizeof(event.x);
    // std::memcpy(&event.y, bytes + offset, sizeof(event.y));
    // offset += sizeof(event.y);
    // std::memcpy(&event.polarity, bytes + offset, sizeof(event.polarity));
    return event;
}

}