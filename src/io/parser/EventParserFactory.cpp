#include "event_lib/io/parser/EventParserFactory.hpp"
#include "event_lib/io/parser/DatParser.hpp"
#include <filesystem>
#include <stdexcept>

namespace event_lib {
    //easy implementation to change if/else implementation for future possible file decoder additions.
    std::unique_ptr<EventParser> EventParserFactory::create_parser(const std::string& path) {
        const std::filesystem::path file_path(path);
        const std::string ext = file_path.extension().string();

        if (ext == ".dat") return std::make_unique<DatParser>();
        if (ext == ".raw") throw std::runtime_error("Raw parser not implemented yet: " + path);
        //return std::make_unique<RawParser>();
        if (ext == ".hdf5" || ext == ".h5") throw std::runtime_error("Hdf5 parser not implemented yet: " + path);
        //return std::make_unique<HdfParser>();
        
        throw std::runtime_error("Unsupported file extension: " + path);
    }
}
