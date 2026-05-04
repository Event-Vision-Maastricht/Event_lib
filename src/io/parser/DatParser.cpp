#include "event_lib/io/parser/DatParser.hpp"
#include <array>
#include <cstdint>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>


namespace {

 //@brief get rid of white space
// std::string trim_copy(const std::string& s) {
//     std::size_t start = 0;
//     while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
//         ++start;
//     }
//     std::size_t end = s.size();
//     while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
//         --end;
//     }
//     return s.substr(start, end - start);
// }
// If lowercase/biggercase matters in the future
// std::string to_lower_copy(const std::string& s) {
//     std::string out;
//     out.reserve(s.size());
//     for (unsigned char ch : s) {
//         out.push_back(static_cast<char>(std::tolower(ch)));
//     }
//     return out;
// }

/**
 * @brief get the exact value after the "keyword"
 */
std::string value_after_keyword(const std::string& raw_line, const std::string& keyword) {
    const std::string line = raw_line;
    // const std::string lower_line = to_lower_copy(line);
    // const std::string lower_keyword = to_lower_copy(keyword);

    //const std::size_t pos = lower_line.find(lower_keyword);
    const std::size_t pos = line.find(keyword);

    if (pos == std::string::npos) {
        return {};
    }

    std::size_t value_start = pos + keyword.size();
    while (value_start < line.size() && (line[value_start] == ' ' || line[value_start] == '\t' ||
                 line[value_start] == ':' || line[value_start] == '=')) ++value_start;

    if (value_start >= line.size()) {
        return {};
    }

    return line.substr(value_start);
}

/**
 * @brief if int is above the limit, return 0, catch any errors
 */
int check_int(const std::string& s) {
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

        // skip binary event metadata block: type(1) + size(1) + reserved(4)
        file_.seekg(6, std::ios::cur);

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
        //each event:
        //      32bit signed + 4 bits + 14 bits +14 bits = 64 bits = 8 bytes total
        std::array<unsigned char, 8> raw_event{};

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

    DatFileHeader DatParser::read_header() {
        DatFileHeader header;

        std::string line;
        while (true) {
            const std::streampos line_start_pos = file_.tellg();
            if (!std::getline(file_, line)) {
                break;
            }
            //end of header keep the pointer at the start of the data
            if (line.empty() || line[0] != '%') {
                file_.clear();
                file_.seekg(line_start_pos, std::ios::beg);
                return header;
            }

            const std::string payload = line.substr(1);

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
                    : d.substr(time_separator + 1);
                    header.date = d.substr(0, time_separator);
                continue;
            }

            std::string h = value_after_keyword(payload, "Height");
            if (!h.empty()) {
                header.height = check_int(h);
                continue;
            }

            std::string w = value_after_keyword(payload, "Width");
            if (!w.empty()) {
                header.width = check_int(w);
                continue;
            }
        }
        return header;
    }

    /**
     * 1- ASCII text header
     * 2- Binary event type and size information
     * 3- Binary event data
     *      data goes in this order:
     *  ts      -       32bit signed (miliseconds)
     *  pol     -       4 bit
     *  y       -       14 bit
     *  x       -       14 bit
     */
    Event DatParser::decode_event(const unsigned char* bytes) const {
        Event event;
        // 4 bytes: packed (polarity:4 | y:14 | x:14)
        std::uint32_t packed = 0;
        // Other 4 bytes: 32-bit signed timestamp (file stores int32)
        std::uint32_t ts32 = 0;

        std::memcpy(&packed, bytes, 4);
        std::memcpy(&ts32, bytes + 4, 4);

        // Assign timestamp into event (widen to long long)
        event.timestamp = static_cast<long long>(ts32);
        
        // Unpack bits (assume polarity in the top 4 bits)
        const std::uint32_t polarity_bits = (packed >> 28) & 0xFu; // 4 bits
        const std::uint32_t y_bits = (packed >> 14) & 0x3FFFu; // next 14 bits
        const std::uint32_t x_bits = packed & 0x3FFFu; // lowest 14 bits

        if(y_bits>header_.height || x_bits>header_.width){
            throw std::runtime_error("Coordinates cannot be bigger than limits");
        }

        event.polarity = polarity_bits != 0;
        event.y = static_cast<int>(y_bits);
        event.x = static_cast<int>(x_bits);

        return event;
    }

}