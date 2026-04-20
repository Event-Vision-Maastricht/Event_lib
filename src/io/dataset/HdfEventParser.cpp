#include "event_lib/io/dataset/HdfEventParser.hpp"
#include <stdexcept>

namespace event_lib {
    std::vector<Event> HdfEventParser::parse(const std::string& path) {
        // TODO: Implement HDF5 parsing with HDF5 library
        throw std::runtime_error("HDF5 parsing not implemented yet: " + path);
    }
}


// //ends header with
// unsigned char ucDataBlock[16] = {
// 	// Offset 0x000023D0 to 0x000023DF
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF
// };
