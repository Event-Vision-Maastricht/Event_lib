#include "event_lib/io/parser/AedatParser.hpp"
#include <array>
#include <algorithm>
#include <cstdint>
#include <cctype>
#include <filesystem>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
/**
 * @brief get the exact value after the "keyword"
 */
std::string value_after_keyword(const std::string& raw_line, const std::string& keyword) {
    const std::string line = raw_line;
    const std::size_t pos = line.find(keyword);

    if (pos == std::string::npos) return {};

    std::size_t value_start = pos + keyword.size();
    while (value_start < line.size() && (line[value_start] == ' ' || line[value_start] == '\t' ||
                 line[value_start] == ':' || line[value_start] == '=')) ++value_start;

    if (value_start >= line.size()) return {};
    return line.substr(value_start);
}

std::string trim_copy(const std::string& s) {
    std::size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    std::size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

std::string to_lower_copy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

bool starts_with(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

std::array<int, 2> geometry_from_aechip(const std::string& aechip) {
    const std::string chip = to_lower_copy(aechip);
    if (chip.find("davis346") != std::string::npos || chip.find("346") != std::string::npos) return {346, 260};
    if (chip.find("davis240") != std::string::npos || chip.find("240") != std::string::npos) return {240, 180};
    if (chip.find("dvs128") != std::string::npos || chip.find("128") != std::string::npos) return {128, 128};
    return {0, 0};
}

std::string month_number(const std::string& month) {
    const std::string m = to_lower_copy(month);
    if (m == "jan") return "01";
    if (m == "feb") return "02";
    if (m == "mar") return "03";
    if (m == "apr") return "04";
    if (m == "may") return "05";
    if (m == "jun") return "06";
    if (m == "jul") return "07";
    if (m == "aug") return "08";
    if (m == "sep") return "09";
    if (m == "oct") return "10";
    if (m == "nov") return "11";
    if (m == "dec") return "12";
    return {};
}

void parse_creation_date(const std::string& value, event_lib::AedatFileHeader& header) {
    // Mon Feb 10 10:57:27 AEDT 2020
    std::istringstream iss(value);
    std::string weekday;
    std::string month;
    std::string day;
    std::string time;
    std::string timezone;
    std::string year;

    if (!(iss >> weekday >> month >> day >> time >> timezone >> year)) {
        header.extra.emplace("creation_date_raw", value);
        return;
    }

    const std::string month_num = month_number(month);
    if (month_num.empty()) {
        header.extra.emplace("creation_date_raw", value);
        return;
    }

    if (day.size() == 1) {
        day.insert(day.begin(), '0');
    }

    header.date = year + "-" + month_num + "-" + day;
    header.time = time;
    //not needed but if needed:
    // header.extra.emplace("creation_date_raw", value);
    // header.extra.emplace("creation_timezone", timezone);
}

/////////////////event parsing helpers////////////////////////////
std::uint32_t read_be32(const unsigned char* bytes) {
    return (static_cast<std::uint32_t>(bytes[0]) << 24) |
           (static_cast<std::uint32_t>(bytes[1]) << 16) |
           (static_cast<std::uint32_t>(bytes[2]) << 8) |
           static_cast<std::uint32_t>(bytes[3]);
}

bool is_davis_aechip(const std::string& aechip) {
    return to_lower_copy(aechip).find("davis") != std::string::npos;
}

} // namespace


/**ASCII header binary event data
 * A header line begins with # and ends with CRLF (Windows line ending, ‘\r\n’).
 * Header lines always case sensitive.
 * very first header line  information about version of AEDAT format.
 *   If it is not present, version 1.0 is assumed.
 * All integer data and fields are big-endian.
 * 
 * 2.0:
 * address field 32 bit
 * header followed by [address (32 bits), timestamp (mmicroseconds 32 bits)] -> 8 bytes per event
 */
//supports aedat 2.0 format
namespace event_lib {

    AedatParser::~AedatParser() {
        close();
    }

    void AedatParser::open(const std::string& path) {
        validate_aedat_path(path);
        close();
        path_ = path;
        file_.open(path_, std::ios::binary);
        if (!file_.is_open()) throw std::runtime_error("Failed to open .aedat file: " + path_);

        file_.seekg(0, std::ios::end);
        const std::streamoff file_size = file_.tellg();
        length_ = file_size > static_cast<std::streamoff>(std::numeric_limits<int>::max())
            ? std::numeric_limits<int>::max()
            : static_cast<int>(file_size);
        file_.seekg(0, std::ios::beg);

        header_ = read_header();
        eof_reached_ = false;
    }

    bool AedatParser::has_more() const {
        return file_.is_open() && !eof_reached_;
    }

    bool AedatParser::reset() {
        if (!file_.is_open()) return false;
        file_.clear();
        file_.seekg(0, std::ios::beg);
        header_ = read_header();
        eof_reached_ = false;
        return true;
    }

    void AedatParser::close() {
        if (file_.is_open()) file_.close();
        eof_reached_ = true;
    }

    const AedatFileHeader& AedatParser::header() const {
        return header_;
    }

    void AedatParser::validate_aedat_path(const std::string& path) {
        const std::filesystem::path p(path);
        if (p.extension() != ".aedat") {
            throw std::runtime_error("AedatParser accepts only .aedat files: " + path);
        }
    }

    int AedatParser::get_length(){
        return length_;
    }

    AedatFileHeader AedatParser::read_header() {
        AedatFileHeader header;
        header.file_format = "aedat";
        std::string line;
        bool in_preferences = false;
        bool saw_supported_version = false;

        if (!std::getline(file_, line)) throw std::runtime_error("Something went wrong while parsing AEDAT file");
        //aedat event version, 2.0 only accepted (!AER-DAT2.0)
        if (starts_with(line, "#!AER-DAT")) {
            const std::string prefix = "#!AER-DAT";
            const std::string version = trim_copy(line.substr(prefix.size()));
            if (version != "2.0") throw std::runtime_error("Only AEDAT 2.0 supported, not " + version);
            header.version = version;
            saw_supported_version = true;
        }else throw std::runtime_error("Only AEDAT 2.0 supported, not 1.0"); // if non existent its 1.0 and not accepted too

        while (true) {
            const std::streampos line_start_pos = file_.tellg();
            if (!std::getline(file_, line)) break;
            if (!line.empty() && line.back() == '\r') line.pop_back();

            if (line.empty() || line[0] != '#') {
                file_.clear();
                file_.seekg(line_start_pos, std::ios::beg);
                return header;
            }
            if (line == "#End Of ASCII Header") return header;

            const std::string payload = trim_copy(line.substr(1));

            //skipping the xml specifications
            if (starts_with(payload, "Start of Preferences for this AEChip")) {
                in_preferences = true;
                continue;
            } if (starts_with(payload, "End of Preferences for this AEChip")) {
                in_preferences = false;
                continue;
            } if (in_preferences) continue;

            //date and time parsing
            std::string creation_date = value_after_keyword(payload, "Creation date");
            if (creation_date.empty() && starts_with(payload, "created")) creation_date = value_after_keyword(payload, "created");
            if (!creation_date.empty()) {
                parse_creation_date(trim_copy(creation_date), header);
                continue;
            }

            //width & height
            std::string aechip = value_after_keyword(payload, "AEChip");
            if (!aechip.empty()) {
                aechip = trim_copy(aechip);
                header.extra.emplace("aechip", aechip);
                const std::array<int, 2> geometry = geometry_from_aechip(aechip);
                header.width = geometry[0];
                header.height = geometry[1];
                continue;
            }
        }
        return header;
    }

    EventPacket AedatParser::read_packet(std::size_t max_events) {
        if (!file_.is_open()) throw std::runtime_error("AedatParser::read_packet called before open().");
        if (max_events == 0)  throw std::runtime_error("max_events must be greater than zero.");
        const std::string aechip = header_.get_extra_or("aechip");
        if (!is_davis_aechip(aechip)) throw std::runtime_error("AEDAT 2.0 event decoding is currently implemented only for DAVIS AEChip data.");

        EventPacket packet;
        Event e;
        // 8 bytes per event, half ts half coordinates
        std::array<unsigned char, 8> raw_word{};
        while(packet.size()< max_events){
            file_.read(reinterpret_cast<char*>(raw_word.data()), 8);
            if(file_.gcount() != 8){
                eof_reached_ = true;
                break;
            }
            if (decode_event(raw_word.data(), e)) {
                packet.add_event(e);
            }
        }
        return packet;
    }

    bool AedatParser::decode_event(const unsigned char* bytes, Event& out_event) const {
        const std::uint32_t address = read_be32(bytes);
        const std::uint32_t timestamp = read_be32(bytes + 4);

        // DAVIS AEDAT2.0 bit 31 == 0 is DVS, bit 31 == 1 is APS or IMU
        const std::uint32_t type = (address >> 31) & 0x1u;
        if (type != 0) return false;

        const std::uint32_t subtype = (address >> 10) & 0x3u;
        if (subtype != 0 && subtype != 2) return false;

        const std::uint32_t y = (address >> 22) & 0x1FFu;
        const std::uint32_t x = (address >> 12) & 0x3FFu;

        if (header_.is_valid() &&
            (x >= static_cast<std::uint32_t>(header_.width) ||
             y >= static_cast<std::uint32_t>(header_.height))) {
            return false;
        }

        out_event.timestamp = static_cast<EventTimestamp>(timestamp);
        out_event.x = static_cast<int>(x);
        out_event.y = header_.is_valid()
            ? header_.height - 1 - static_cast<int>(y)
            : static_cast<int>(y);
        out_event.polarity = subtype == 2;
        return true;
    }

}
