#include "event_lib/io/parser/RawParser.hpp"
#include <array>
#include <cstdint>
#include <cctype>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
    constexpr uint32_t EVT_TIME_HIGH = 0x8;
    constexpr uint32_t CD_OFF        = 0x0;
    constexpr uint32_t CD_ON         = 0x1;
    constexpr uint32_t EXT_TRIGGER = 0xA;
    constexpr uint32_t OTHERS = 0xE;
    constexpr uint32_t CONTINUED = 0xF;

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

uint32_t read_le32(const unsigned char* bytes) {
    return static_cast<uint32_t>(bytes[0]) |
           (static_cast<uint32_t>(bytes[1]) << 8) |
           (static_cast<uint32_t>(bytes[2]) << 16) |
           (static_cast<uint32_t>(bytes[3]) << 24);
}
} // namespace

//TODO: future recommendation: evt 2.1 format support
namespace event_lib {
    RawParser::~RawParser(){close();}

    void RawParser::open(const std::string& path){
        validate_raw_path(path);
        close();

        path_ = path;
        file_.open(path_,std::ios::binary);
        if(!file_.is_open()) throw std::runtime_error("Failed to open .raw file: " + path_);

        file_.seekg(0, std::ios::end);
        const std::streamoff file_size = file_.tellg();
        length_ = file_size > static_cast<std::streamoff>(std::numeric_limits<int>::max())
            ? std::numeric_limits<int>::max()
            : static_cast<int>(file_size);
        file_.seekg(0, std::ios::beg);

        header_ = read_header();
        eof_reached_ = false;
    }

    bool RawParser::has_more() const{
        return file_.is_open() && !eof_reached_;
    }

    // Event data written in binary little or big-endian (depending on the sensor configuration, little-endian by default
    EventPacket RawParser::read_packet(std::size_t max_events){
        if (!file_.is_open()) throw std::runtime_error("RawParser::read_packet called before open().");
        if (max_events == 0) throw std::runtime_error("max_events must be greater than zero.");

        std::string val = header_.get_extra_or("evt");
        if ( val != "2.0"){
            throw std::runtime_error("Only EVT2.0 is currently supported, not " + val);
        }
        //each event in 2.0 format:   https://docs.prophesee.ai/stable/data/encoding_formats/evt2.html#chapter-data-encoding-formats-evt2
        //timestamp in microsecond, 34 bits, rollout at 2^34microsec
        std::array<unsigned char, 4> raw_word{};

        EventPacket packet;
        Event e;
        while(packet.size()< max_events){
            file_.read(reinterpret_cast<char*>(raw_word.data()), 4);
            if(file_.gcount() != 4){
                eof_reached_ = true;
                break;
            }
            if(decode_event(raw_word.data(),e)) packet.add_event(e);
        }
        return packet;
    }

        bool RawParser::decode_event(const unsigned char* bytes, Event& out_event) {
        const uint32_t word = read_le32(bytes);

        const uint32_t type = (word >> 28) & 0xF;

        // 4 MSB are to define word type. could be CD_OFF, CD_ON, EVT_TIME_HIGH, EXT_TRIGGER, OTHERS, CONTINUED
        switch(type) {
            case EVT_TIME_HIGH: {
                current_time_high_ = word & 0x0FFFFFFF;
                return false;
            }
            case CD_OFF:
            case CD_ON: {
                const uint32_t ts_low = (word >> 22) & 0x3F;
                const uint32_t x = (word >> 11) & 0x7FF;
                const uint32_t y = word & 0x7FF;
                const uint64_t timestamp = (static_cast<uint64_t>(current_time_high_) << 6)| ts_low;
                out_event.x = static_cast<int>(x);
                out_event.y = static_cast<int>(y);
                out_event.timestamp = static_cast<EventTimestamp>(timestamp);
                out_event.polarity = (type == CD_ON);
                return true;
            }
            case EXT_TRIGGER: {
                return false;
            }
            case OTHERS: {
                return false;
            }
            case CONTINUED: { //continued extends whatever was before
                return false;
            }
            default:
                return false;
        }
    }

    /**
     * % Date 2020-09-14 09:03:25
     * % firmware_version 2.0.2
     * % integrator_name Prophesee
     * % plugin_name hal_plugin_gen3_fx3
     * % serial_number 30384338
     * % system_ID 21
     * % evt 2.0
     * 
     * prophesee  header example:
     * 
     * % camera_integrator_name Prophesee
     * % date 2023-03-29 16:37:46
     * % evt 3.0
     * % format EVT3;height=720;width=1280
     * % generation 4.2
     * % geometry 1280x720
     * % integrator_name Prophesee
     * % plugin_integrator_name Prophesee
     * % plugin_name hal_plugin_imx636_evk4
     * % sensor_generation 4.2
     * % serial_number 00ca0009
     * % system_ID 49
     * % end
     */
    RawFileHeader RawParser::read_header() {
        RawFileHeader header;
        header.file_format = "raw";

        std::string line;
        while (true) {
            const std::streampos line_start_pos = file_.tellg();
            if (!std::getline(file_, line)) break;

            if (line.empty() || line[0] != '%' || line == "% end") {
                file_.clear();
                file_.seekg(line_start_pos, std::ios::beg);
                return header;
            }

            const std::string payload = line.substr(1);
            
            std::string d = value_after_keyword(payload, "Date");
            // % Date 2020-09-14 09:03:25
            if (!d.empty()) {
                const std::size_t time_separator = d.find(' ');
                header.time = time_separator == std::string::npos
                    ? std::string{}
                    : d.substr(time_separator + 1);
                    header.date = d.substr(0, time_separator);
                continue;
            }

            std::string firmware = value_after_keyword(payload, "firmware_version");
            //% firmware_version 2.0.2
            if (!firmware.empty()) {
                header.extra.emplace("firmware_version", firmware);
                continue;
            }

            std::string integrator = value_after_keyword(payload, "integrator_name");
            // % integrator_name Prophesee
            if (!integrator.empty()) {
                header.extra.emplace("integrator_name", integrator);
                continue;
            }
            
            std::string serialnum = value_after_keyword(payload, "serial_number");
            // % serial_number 30384338
            if (!serialnum.empty()) {
                header.extra.emplace("serial_number", serialnum);
                continue;
            }

            std::string sysid = value_after_keyword(payload, "system_ID");
            // % system_ID 21
            if (!sysid.empty()) {
                header.extra.emplace("system_id", sysid);
                continue;
            }

            std::string evt = value_after_keyword(payload, "evt");
            // % evt 2.0
            if (!evt.empty()) {
                header.extra.emplace("evt", evt);
                continue;
            }

            std::string camera = value_after_keyword(payload, "plugin_name");
            // % plugin_name hal_plugin_gen3_fx3
            if (!camera.empty()) {
                header.extra.emplace("plugin_name", camera);
                std::array<int,2> r = find_geometry(camera);
                header.width = r[0];
                header.height = r[1];
                continue;
            }
        }

        return header;
    }

    // | Sensor            | Resolution |
    // | ----------------- | ---------- |
    // | Gen3              | 640×480    |
    // | Gen4 HD           | 1280×720  -> evk4 resolution true data
    // | DAVIS346          | 346×260    |
    // | DVXplorer         | 640×480    |
    // | Prophesee GenX320 | 320×320    |
    std::array<int, 2> RawParser::find_geometry(const std::string& cameraName){
        //default since this is the camera used by uni
        std::array<int,2> r = {1280,720};
        if (cameraName.empty()) return r;
        std::string s = cameraName;
        // to lower
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        if ((s.find("gen3") != std::string::npos || s.find("prophesee") != std::string::npos) && s.find("320") == std::string::npos) {
            r = {640,480};
            return r;
        }
        if (s.find("gen4") != std::string::npos || s.find("evk4") != std::string::npos || s.find("hd") != std::string::npos) {
            r = {1280,720};
            return r;
        }
        if (s.find("davis") != std::string::npos || s.find("346") != std::string::npos) {
            r = {346,260};
            return r;
        }
        if (s.find("dvxplorer") != std::string::npos) {
            r = {640,480};
            return r;
        }
        if (s.find("genx320") != std::string::npos || s.find("320") != std::string::npos) {
            r = {320,320};
            return r;
        }
        

        return r;
    }
    
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