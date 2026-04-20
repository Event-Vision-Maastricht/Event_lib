#include "event_lib/io/parser/FileParser.hpp"
#include "event_lib/io/parser/DatParser.hpp"
#include <stdexcept>
#include <filesystem>

namespace event_lib {
    FileType FileParser::file_type(const std::string& path) {
        const std::filesystem::path file_path(path);
        const std::string extension = file_path.extension().string();

        if (extension == ".raw") return FileType::Raw;
        if (extension == ".dat") return FileType::Dat;
        if (extension == ".hdf5" || extension == ".h5") return FileType::Hdf5;
        return FileType::Unknown;
    }

    bool FileParser::is_supported(const std::string& path) {
        return file_type(path) == FileType::Dat;
    }

    std::vector<Event> FileParser::load_events(const std::string& path) {
        switch (file_type(path)) {
            case FileType::Dat:
                return DatParser::parse(path);
            case FileType::Raw:
                throw std::runtime_error("Raw File not supported: " + path);
            case FileType::Hdf5:
                throw std::runtime_error("Hdf5 file not supported: " + path);
            default:
                throw std::runtime_error("Unsupported file extension: " + path);
        }
    }
}