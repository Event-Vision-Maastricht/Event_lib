#pragma once

#include <string>

namespace event_lib {
    /**
     * @brief Unified sensor metadata structure shared across all modules.
     * 
     * This struct contains all metadata about the recording sensor and format.
     * It's used by parsers, streams, processing modules, and visualization.
     * 
     * This centralized definition prevents duplication and ensures consistency
     * across the entire library when header information needs to be accessed.
     */
    struct SensorMetadata {
        // Sensor dimensions
        int width = 0;                      ///< Horizontal size of image sensor array
        int height = 0;                     ///< Vertical size of image sensor array
        
        // Recording metadata
        std::string date;                   ///< Recording date, format: YYYY-MM-DD
        std::string time;                   ///< Recording time, format: HH:MM:SS
        std::string version;                ///< Format version
        std::string event_type;             ///< Type of event: CD/2d/ExtTrig
        
        /**
         * @brief Virtual destructor for polymorphic use if needed.
         */
        virtual ~SensorMetadata() = default;
        
        /**
         * @brief Check if metadata has been properly initialized.
         * @return true if width and height are > 0
         */
        bool is_valid() const {
            return width > 0 && height > 0;
        }
    };
    
    /**
     * @brief Alias for backward compatibility with FileHeader naming.
     * 
     * Use SensorMetadata directly in new code.
     */
    using FileHeader = SensorMetadata;
}
