#include "event_lib/processing/Frame.hpp"
#include <algorithm>
#include <iostream>
#include <utility>

namespace event_lib {

    Frame::Frame(int width, int height)
        : width_(width),
          height_(height),
          total_events_(0),
          on_events_count_(0),
          off_events_count_(0) {
        ensure_frame_storage(current_frame_);
        current_frame_.timestamp = 0;
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
        if (polarity) {
            current_frame_.on_events[y][x]++;
            ++on_events_count_;
        } else {
            current_frame_.off_events[y][x]++;
            ++off_events_count_;
        }
        ++total_events_;
    }

    bool Frame::finalize_frame(long timestamp, FrameStr& output_frame) {
        if (total_events_ == 0) {
            return false;
        }

        current_frame_.timestamp = timestamp;
        ensure_frame_storage(output_frame);

        std::swap(output_frame.on_events, current_frame_.on_events);
        std::swap(output_frame.off_events, current_frame_.off_events);
        output_frame.timestamp = timestamp;

        // Reuse swapped-in storage for the next frame instead of reallocating.
        current_frame_.timestamp = 0;
        reset_frame();

        return true;
    }

    void Frame::reset_frame() {
        for (int y = 0; y < height_; ++y) {
            std::fill(current_frame_.on_events[y].begin(), 
                      current_frame_.on_events[y].end(), 0);
            std::fill(current_frame_.off_events[y].begin(), 
                      current_frame_.off_events[y].end(), 0);
        }
        total_events_ = 0;
        on_events_count_ = 0;
        off_events_count_ = 0;
    }

    int Frame::get_total_events() const {
        return total_events_;
    }

    int Frame::get_on_events() const {
        return on_events_count_;
    }

    int Frame::get_off_events() const {
        return off_events_count_;
    }

    void Frame::ensure_frame_storage(FrameStr& frame) {
        if (static_cast<int>(frame.on_events.size()) != height_) {
            frame.on_events.assign(height_, std::vector<int>(width_, 0));
        } else {
            for (int y = 0; y < height_; ++y) {
                if (static_cast<int>(frame.on_events[y].size()) != width_) {
                    frame.on_events[y].assign(width_, 0);
                }
            }
        }

        if (static_cast<int>(frame.off_events.size()) != height_) {
            frame.off_events.assign(height_, std::vector<int>(width_, 0));
        } else {
            for (int y = 0; y < height_; ++y) {
                if (static_cast<int>(frame.off_events[y].size()) != width_) {
                    frame.off_events[y].assign(width_, 0);
                }
            }
        }
    }

}
