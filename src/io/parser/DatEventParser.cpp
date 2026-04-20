#include "event_lib/io/parser/DatParser.hpp"
#include <fstream>
#include <filesystem>
#include <stdexcept>

namespace event_lib {
    void DatParser::validate_dat_path(const std::string& path) {
        const std::filesystem::path p(path);
        if (p.extension() != ".dat") {
            throw std::runtime_error("DatParser accepts only .dat files: " + path);
        }
    }

    DatFileHeader DatParser::read_header(const std::string& path) {
        validate_dat_path(path);
        DatFileHeader header;

        // Placeholder until the full DAT header spec is integrated.
        // Keep defaults and fill as parser support matures.
        return header;
    }

    std::vector<Event> DatParser::parse(const std::string& path) {
        validate_dat_path(path);

        std::vector<Event> events;
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open .dat file: " + path);
        }

        // TODO: Implement full DAT event decoding once packet layout is finalized.
        return events;
    }

}