#include "event_lib/io/dataset/FileParser.hpp"
#include "event_lib/io/dataset/DatEventParser.hpp"
#include "event_lib/io/dataset/RawEventParser.hpp"
#include "event_lib/io/dataset/HdfEventParser.hpp"
#include <stdexcept>
#include <filesystem>

namespace event_lib {
    int FileParser::file_type(const std::string& path) {
        std::filesystem::path filePath(path);
        std::string extension = filePath.extension().string();
        if (extension == ".raw") return 1;
        if (extension == ".dat") return 2;
        if (extension == ".hdf5" || extension == ".h5") return 3;
        return 0; // unknown
    }

    std::vector<Event> FileParser::load_events(const std::string& path) {
        int type = file_type(path);
        switch (type) {
            case 1: // raw
                return RawEventParser::parse(path);
            case 2: // dat
                return DatEventParser::parse(path);
            case 3: // hdf5
                return HdfEventParser::parse(path);
            default:
                throw std::runtime_error("Unsupported file format: " + path);
        }
    }
}