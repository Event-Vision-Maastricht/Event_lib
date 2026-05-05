#include <string>
#include <vector>
#include <fstream>
#include <cstring>

#include "../core/event.hpp"

namespace event_lib{
    // Frame representation with polarity support
    struct FrameStr {
        std::vector<std::vector<int>> on_events;   // Polarity ON (true)
        std::vector<std::vector<int>> off_events;  // Polarity OFF (false)
        long timestamp;
    };

    // Frame Packet: efficient container for multiple frames
    // Batches 3600 completed frames for video generation
    struct FramePacket {
        static constexpr int FRAME_CAPACITY = 3600;  // 3600 frames per packet
        
        std::vector<FrameStr> frames;
        int width;
        int height;
        long start_timestamp;
        long end_timestamp;

        FramePacket() : width(0), height(0), start_timestamp(0), end_timestamp(0) {}
        
        FramePacket(int w, int h) : width(w), height(h), start_timestamp(0), end_timestamp(0) {}

        // Add a completed frame to packet
        void add_frame(const FrameStr& frame) {
            frames.push_back(frame);
            if (frames.size() == 1) {
                start_timestamp = frame.timestamp;
            }
            end_timestamp = frame.timestamp;
        }

        // Check if packet is full (3600 frames)
        bool is_full() const {
            return frames.size() >= FRAME_CAPACITY;
        }

        // Get current frame count
        size_t frame_count() const { return frames.size(); }

        // Clear all frames
        void clear() {
            frames.clear();
            start_timestamp = 0;
            end_timestamp = 0;
        }
    };

    // Efficient frame generator for event batching
    class Frame {
    public:
        Frame(int width, int height);
        ~Frame();

        // Add an event to the current frame
        void add_event(const Event& ev);

        // Finalize current frame and add to packet
        // Returns true if packet becomes full after adding this frame
        bool finalize_frame(long timestamp);

        // Get reference to current frame (before finalization)
        const FrameStr& get_current_frame() const { return current_frame_; }

        // Get the current packet (non-destructive)
        const FramePacket& get_current_packet() const { return frame_packet_; }

        // Get and clear the current packet (destructive - use for streaming to video)
        FramePacket extract_packet() {
            FramePacket temp = frame_packet_;
            frame_packet_.clear();
            return temp;
        }

        // Check if current packet is full
        bool is_packet_full() const {
            return frame_packet_.is_full();
        }

        // Get packet frame count
        size_t get_packet_frame_count() const {
            return frame_packet_.frame_count();
        }

        // Reset current frame (clear all counters)
        void reset_frame();

        // Get frame statistics
        int get_total_events() const;
        int get_on_events() const;
        int get_off_events() const;

    private:
        FrameStr current_frame_;
        FramePacket frame_packet_;
        int width_;
        int height_;
    };
}




/*
    bool FrameVisualizer::save_frame(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << " for writing\n";
            return false;
        }

        // Write header
        file.write(reinterpret_cast<const char*>(&width_), sizeof(int));
        file.write(reinterpret_cast<const char*>(&height_), sizeof(int));
        file.write(reinterpret_cast<const char*>(&current_frame_.timestamp), sizeof(long));

        // Write ON events
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                int val = current_frame_.on_events[y][x];
                file.write(reinterpret_cast<const char*>(&val), sizeof(int));
            }
        }

        // Write OFF events
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                int val = current_frame_.off_events[y][x];
                file.write(reinterpret_cast<const char*>(&val), sizeof(int));
            }
        }

        file.close();
        return true;
    }

    bool FrameVisualizer::save_frame_as_ppm(const std::string& filename, bool use_polarity_color) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << " for writing\n";
            return false;
        }

        // PPM header (P3 = ASCII color, P6 = binary color)
        file << "P3\n";
        file << width_ << " " << height_ << "\n";
        file << "255\n";

        // Find max value for normalization
        int max_val = 1;
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                max_val = std::max(max_val, current_frame_.on_events[y][x]);
                max_val = std::max(max_val, current_frame_.off_events[y][x]);
            }
        }

        // Write pixels
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                int on_val = current_frame_.on_events[y][x];
                int off_val = current_frame_.off_events[y][x];

                int r, g, b;
                if (use_polarity_color) {
                    // Red for ON, Blue for OFF
                    r = static_cast<int>(255.0 * on_val / max_val);
                    g = 0;
                    b = static_cast<int>(255.0 * off_val / max_val);
                } else {
                    // Grayscale (combined)
                    int combined = on_val + off_val;
                    int gray = static_cast<int>(255.0 * combined / (2 * max_val));
                    r = g = b = gray;
                }

                file << r << " " << g << " " << b << "\n";
            }
        }

        file.close();
        return true;
    }
*/