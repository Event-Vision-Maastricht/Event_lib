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

    void RawParser::open(const std::string& path){
        return;
    }

    bool RawParser::has_more() const{
        return false;
    }

    EventPacket RawParser::read_packet(std::size_t max_events){
        EventPacket packet;
        return packet;
    }

    /**
     * % Date 2020-09-14 09:03:25
     * % firmware_version 2.0.2
     * % integrator_name Prophesee
     * % plugin_name hal_plugin_gen3_fx3
     * % serial_number 30384338
     * % system_ID 21
     * % evt 2.0
     */
    RawFileHeader RawParser::read_header() {
        RawFileHeader header;
        header.file_format = "raw";

        std::string line;
        while (true) {
            const std::streampos line_start_pos = file_.tellg();
            if (!std::getline(file_, line)) break;

            if (line.empty() || line[0] != '%') {
                file_.clear();
                file_.seekg(line_start_pos, std::ios::beg);
                return header;
            }

            const std::string payload = line.substr(1);
            std::size_t sp = payload.find(' ');
            std::string key, val;
            if (sp != std::string::npos) {
                key = payload.substr(0, sp);
                val = payload.substr(sp + 1);
                if (!val.empty() && val.back() == '.') val.pop_back();
            } else {
                key = payload;
                val.clear();
            }

            if (key == "Date") {
                const std::size_t time_separator = val.find(' ');
                header.time = time_separator == std::string::npos ? std::string{} : val.substr(time_separator + 1);
                header.date = val.substr(0, time_separator);
                header.extra.emplace(key, val);
                continue;
            }

            if (key == "Width") {
                header.width = check_int(val);
                header.extra.emplace(key, val);
                continue;
            }

            if (key == "Height") {
                header.height = check_int(val);
                header.extra.emplace(key, val);
                continue;
            }

            if (key == "evt") {
                header.version = val;
                header.extra.emplace(key, val);
                continue;
            }

            if(key == "plugin_name"){
                std::array<int,2> r = find_geometry(val);
                header.width = r[0];
                header.height = r[1];
                continue;
                
            }

            header.extra.emplace(key, val);
        }

        return header;
    }

    /////////////source: chatgpt !!!!!!! non trustable !!!!!!
    // | Sensor            | Resolution |
    // | ----------------- | ---------- |
    // | Gen3              | 640×480    |
    // | Gen4 HD           | 1280×720  -> evk4 resolution true data
    // | DAVIS346          | 346×260    |
    // | DVXplorer         | 640×480    |
    // | Prophesee GenX320 | 320×320    |
    /**
     * possible names to cameras:
     * hal_plugin_gen3_fx3
     * hal_plugin_gen41_evk3
     */
    std::array<int, 2> RawParser::find_geometry(const std::string& cameraName){
        //default since this is the camera used by uni
        std::array<int,2> r = {1280,720};
        if (cameraName.empty()) return r;
        std::string s = cameraName;
        // to lower
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        if (s.find("gen3") != std::string::npos || s.find("prophesee") != std::string::npos && s.find("320") == std::string::npos) {
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

    Event RawParser::decode_event(const unsigned char* bytes) const {
        Event e;
        return e;
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


