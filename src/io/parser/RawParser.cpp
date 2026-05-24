#include "event_lib/io/parser/RawParser.hpp"
#include <fstream>
#include <stdexcept>

namespace {

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
        const long parsed = std::stoll(s);
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
    RawParser::~RawParser(){close();}

    void RawParser::open(const std::string& path){}

    bool RawParser::has_more() const{}

    EventPacket RawParser::read_packet(std::size_t max_events){}

    /**
     * % Date 2020-09-14 09:03:25
     * % firmware_version 2.0.2
     * % integrator_name Prophesee
     * % plugin_name hal_plugin_gen3_fx3
     * % serial_number 30384338
     * % system_ID 21
     * % evt 2.0
     */
    RawFileHeader RawParser::read_header() {}

    Event RawParser::decode_event(const unsigned char* bytes) const {}
    
    bool RawParser::reset() {
        if (!file_.is_open()) return false;
        file_.clear();
        file_.seekg(0, std::ios::beg);
        header_ = read_header();
        eof_reached_ = false;
        return true;
    }

    void RawParser::close() {
        if (file_.is_open()) file_.close();
        eof_reached_ = true;
    }

    const RawFileHeader& RawParser::header() const {
        return header_;
    }

    void RawParser::validate_raw_path(const std::string& path) {
        const std::filesystem::path p(path);
        if (p.extension() != ".raw") {
            throw std::runtime_error("RawParser accepts only .raw files: " + path);
        }
    }

    int RawParser::get_length(){
        return length_;
    }

}


