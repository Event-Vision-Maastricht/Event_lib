#pragma once

#include <condition_variable>
#include <deque>
#include <fstream>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

#include "event_lib/core/event.hpp"
#include "event_lib/core/sensor_metadata.hpp"

namespace event_lib{
    // Frame representation with polarity support
    struct FrameStr {
        std::vector<std::vector<int>> on_events;   // Polarity ON (true)
        std::vector<std::vector<int>> off_events;  // Polarity OFF (false)
        long timestamp;
    };

    // Efficient frame generator for event batching
    class Frame {
    public:
        Frame(const SensorMetadata& metadata);
        ~Frame();

        // Add an event to the current frame
        void add_event(const Event& ev);

        bool finalize_frame(long timestamp, FrameStr& output_frame);

        // Get reference to current frame (before finalization)
        const FrameStr& get_current_frame() const { return current_frame_; }

        // Reset current frame (clear all counters)
        void reset_frame();

        // Get frame statistics
        int get_total_events() const;
        int get_on_events() const;
        int get_off_events() const;

    private:
        void ensure_frame_storage(FrameStr& frame);

        const SensorMetadata* metadata_{nullptr};
        FrameStr current_frame_;
        int total_events_;
        int on_events_count_;
        int off_events_count_;
    };
}