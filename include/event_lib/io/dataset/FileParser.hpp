#pragma once
#include <string>
#include <vector>
#include "../../core/event.hpp"

namespace event_lib {
    class FileParser {
    public:
        /**
         * @brief Detects file type: 1: raw, 2: dat, 3: hdf5
         * @param path path of the file
         * @return 1: raw, 2: dat, 3: hdf5, 0: unknown
         */
        static int file_type(const std::string& path);

        /**
         * @brief Loads events from file based on detected format
         * @param path path of the file
         * @return vector of Events
         */
        static std::vector<Event> load_events(const std::string& path);
    };
}