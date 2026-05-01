#include "event_lib/io/parser/DatParser.hpp"
#include <array>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>


namespace {

std::string trim_copy(const std::string& s) {
    std::size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }

    std::size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }

    return s.substr(start, end - start);
}

std::string to_lower_copy(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char ch : s) {
        out.push_back(static_cast<char>(std::tolower(ch)));
    }
    return out;
}

std::string value_after_keyword(const std::string& raw_line, const std::string& keyword) {
    const std::string line = trim_copy(raw_line);
    const std::string lower_line = to_lower_copy(line);
    const std::string lower_keyword = to_lower_copy(keyword);

    const std::size_t pos = lower_line.find(lower_keyword);
    if (pos == std::string::npos) {
        return {};
    }

    std::size_t value_start = pos + keyword.size();
    while (value_start < line.size() &&
           (line[value_start] == ' ' || line[value_start] == '\t' || line[value_start] == ':' || line[value_start] == '=')) {
        ++value_start;
    }

    if (value_start >= line.size()) {
        return {};
    }

    return trim_copy(line.substr(value_start));
}

int parse_positive_int_or_zero(const std::string& s) {
    try {
        const long long parsed = std::stoll(s);
        if (parsed < 0 || parsed > std::numeric_limits<int>::max()) {
            return 0;
        }
        return static_cast<int>(parsed);
    } catch (...) {
        return 0;
    }
}

} // namespace

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

    file_.seekg(0, std::ios::end);
    const std::streamoff file_size = file_.tellg();
    length_ = file_size > static_cast<std::streamoff>(std::numeric_limits<int>::max())
        ? std::numeric_limits<int>::max()
        : static_cast<int>(file_size);
    file_.seekg(0, std::ios::beg);

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

int DatParser::get_length(){
    return length_;
}

//                  Example header:
//             % Data file containing CD events.
//             % Version 2
//             % Date 2020-09-14 16:03:08
//             % Height 480
//             % Width 640
DatFileHeader DatParser::read_header() {
    DatFileHeader header;
    // if (!file_.is_open()) {
    //     return header;
    // }
    // file_.clear();
    // file_.seekg(0, std::ios::beg);

    std::string line;
    while (true) {
        const std::streampos line_start_pos = file_.tellg();
        if (!std::getline(file_, line)) {
            break;
        }
        //end of header keep the pointer at the start of the data
        if (line.empty() || line[0] != '%') {
            // file_.clear();
            // file_.seekg(line_start_pos, std::ios::beg);
            return header;
        }

        const std::string payload = trim_copy(line.substr(1));

        // % Data file containing CD events.
        std::string  t = value_after_keyword(payload, "Data file containing");
        if (!t.empty()) {
            std::size_t space_pos = t.find(' ');
            if (space_pos != std::string::npos) t = t.substr(0, space_pos);
            if(t != "CD") throw std::runtime_error("Only CD type of events, not " + t);
            header.event_type = t;
            continue;
        }
        
        std::string v = value_after_keyword(payload, "Version");
        if (!v.empty()) {
            header.version = v;
            continue;
        }

        std::string d = value_after_keyword(payload, "Date");
        //  % Date 2020-09-14 16:03:08
        if (!d.empty()) {
            const std::size_t time_separator = d.find(' ');
            header.time = time_separator == std::string::npos
                ? std::string{}
                : trim_copy(d.substr(time_separator + 1));
                header.date = d.substr(0, time_separator);
            continue;
        }

        std::string h = value_after_keyword(payload, "Height");
        if (!h.empty()) {
            header.height = parse_positive_int_or_zero(h);
            continue;
        }

        std::string w = value_after_keyword(payload, "Width");
        if (!w.empty()) {
            header.width = parse_positive_int_or_zero(w);
            continue;
        }

        // DatFileHeader::type = value_after_keyword(payload, "event_type");
        // if (!type.empty()) {
        //     header.event_type = type;
        //     continue;
        // }
    }

    return header;
}
/**
 * 1- ASCII text header
 * 2- Binary event type and size information
 * 3- Binary event data
 */
Event DatParser::decode_event(const unsigned char* bytes) const {
    Event event;
    std::size_t offset = 0;

    std::memcpy(&event.timestamp, bytes + offset, sizeof(event.timestamp));
    offset += sizeof(event.timestamp);

    std::memcpy(&event.x, bytes + offset, sizeof(event.x));
    offset += sizeof(event.x);

    std::memcpy(&event.y, bytes + offset, sizeof(event.y));
    offset += sizeof(event.y);

    std::memcpy(&event.polarity, bytes + offset, sizeof(event.polarity));

    return event;
}

}