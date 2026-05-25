#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <cstdint>

namespace event_lib {
    /**
     * @brief Unified sensor metadata structure shared across all modules.
     *
     * This struct contains a small set of canonical fields consumers rely on
     * (notably sensor geometry) and a lossless `extra` map that parsers can
     * populate with format-specific header key/value pairs.
     */
    struct SensorMetadata {
        // Sensor dimensions
        int width = 0;                          ///< Horizontal size of image sensor array
        int height = 0;                         ///< Vertical size of image sensor array
        std::string file_format;                ///<raw, dat, aedat etc.
        std::string date;                       ///< Recording date, format: YYYY-MM-DD
        std::string time;                       ///< Recording time, format: HH:MM:SS
        
        // Recording metadata FOR DAT
        std::optional<std::string> version;     ///< Format version
        std::optional<std::string> event_type;  ///< Type of event: CD/2d/ExtTrig

        // // Recording metadata FOR RAW
        // std::optional<std::string> firmware_version;
        // std::optional<std::string> integrator_name;
        // std::optional<std::string> plugin_name;
        // std::optional<uint16_t> serial_number;
        // std::optional<int> system_ID;
        // std::optional<int> evt;

        std::unordered_map<std::string, std::string> extra;

        /**
         * @brief Virtual destructor for polymorphic use if needed.
         */
        virtual ~SensorMetadata() = default;

        /**
         * @brief Check if the geometry has been initialized.
         * @return true if width and height are > 0
         */
        bool is_valid() const { return width > 0 && height > 0; }

        // Helpers for `extra`
        bool has_extra(const std::string& key) const {
            return extra.find(key) != extra.end();
        }

        std::string get_extra_or(const std::string& key, const std::string& fallback = "") const {
            auto it = extra.find(key);
            return it == extra.end() ? fallback : it->second;
        }
    };

}
