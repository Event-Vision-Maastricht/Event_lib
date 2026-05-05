#include "event_lib/processing/Frame.hpp"
#include "event_lib/core/event.hpp"
#include <algorithm>
#include <iostream>

namespace event_lib {

    Frame::Frame(int width, int height) 
        : width_(width), height_(height) {
        
        current_frame_.timestamp = 0;
        current_frame_.on_events.resize(height, std::vector<int>(width, 0));
        current_frame_.off_events.resize(height, std::vector<int>(width, 0));
        
        frame_packet_.width = width;
        frame_packet_.height = height;
    }

    Frame::~Frame() {/* Cleanup if needed*/}

    void Frame::add_event(const Event& ev) {
        int x = ev.x;
        int y = ev.y;
        bool polarity = ev.polarity;

        // Bounds checking
        if (x < 0 || x >= width_ || y < 0 || y >= height_) {
            std::cerr << "Coordinates cannot exceed limits. (Frame.cpp)"<< std::endl;
            return;
        }

        // Add to appropriate polarity frame
        if (polarity) current_frame_.on_events[y][x]++;
        else current_frame_.off_events[y][x]++;
    }

    bool Frame::finalize_frame(long timestamp) {
        // Set timestamp for the finalized frame
        current_frame_.timestamp = timestamp;

        // Add completed frame to packet
        frame_packet_.add_frame(current_frame_);

        // Reset for next frame
        reset_frame();

        // Check if packet is full
        return frame_packet_.is_full();
    }

    void Frame::reset_frame() {
        for (int y = 0; y < height_; ++y) {
            std::fill(current_frame_.on_events[y].begin(), 
                      current_frame_.on_events[y].end(), 0);
            std::fill(current_frame_.off_events[y].begin(), 
                      current_frame_.off_events[y].end(), 0);
        }
    }

    int Frame::get_total_events() const {
        int total = 0;
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                total += current_frame_.on_events[y][x];
                total += current_frame_.off_events[y][x];
            }
        }
        return total;
    }

    int Frame::get_on_events() const {
        int total = 0;
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                total += current_frame_.on_events[y][x];
            }
        }
        return total;
    }

    int Frame::get_off_events() const {
        int total = 0;
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                total += current_frame_.off_events[y][x];
            }
        }
        return total;
    }

}
