#pragma once

#include <condition_variable>
#include <chrono>
#include <fstream>
#include <cstring>
#include <mutex>
#include <memory>
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
        
        void decay_frame(double amount);

        bool finalize_frame(long timestamp, FrameStr& output_frame);
        bool publish_frame(long timestamp, double decay_amount = 0.85);
        bool consume_published_frame(FrameStr& output_frame);
        bool wait_for_published_frame(std::chrono::milliseconds timeout) const;
        void close();

        // Get reference to current frame (before finalization)
        const FrameStr& get_current_frame() const { return current_frame_; }

        // Reset current frame (clear all counters)
        void reset_frame();

        // Get-setters frame statistics
        int get_total_events() const;
        int get_on_events() const;
        int get_off_events() const;
        bool is_dirty() const;
        bool consume_dirty();
        void set_dirty(bool newDirty);
        long get_last_update() const;
        const SensorMetadata& get_metadata() const;

    private:
        void ensure_frame_storage(FrameStr& frame);
        bool finalize_frame_unlocked(long timestamp, FrameStr& output_frame);
        bool publish_frame_unlocked(long timestamp, FrameStr& output_frame);
        void decay_frame_unlocked(double amount);

        const SensorMetadata* metadata_{nullptr};
        FrameStr current_frame_;
        FrameStr published_frame_;
        int total_events_;
        int on_events_count_;
        int off_events_count_;
        bool is_dirty_ = false;
        bool has_published_frame_ = false;
        bool closed_ = false;
        mutable std::mutex mutex_;
        mutable std::condition_variable frame_ready_;
        mutable std::condition_variable publish_slot_free_;
    };
}
