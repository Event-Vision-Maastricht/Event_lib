#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include "event_lib/processing/DisplayMode.hpp"
#include <algorithm>
#include <iostream>

////////////////LAB2B HAS THE TIME WINDOW HISTOGRAM IMPLEMENTATION
namespace event_lib {

    bool DisplayMode::init_metadata(const SensorMetadata& metadata){
        metadata_ = &metadata;
        frame_ = std::make_shared<Frame>(*metadata_);
        stop_requested_ = std::make_shared<std::atomic<bool>>(false);
        event_counter_ = 0;
        last_event_timestamp_ = 0;
        time_window_start_ = 0;
        time_window_end_ = 0;
        has_time_window_ = false;
        has_pending_events_ = false;
        initialized = true;
        return true;
    }

    std::shared_ptr<Frame> DisplayMode::get_frame() const {
        return frame_;
    }

    std::shared_ptr<std::atomic<bool>> DisplayMode::get_stop_flag() const {
        return stop_requested_;
    }

    bool DisplayMode::flush_pending_frame(){
        if (!initialized || !stop_requested_ || stop_requested_->load() || !frame_ || !has_pending_events_) return false;

        const bool published = frame_->publish_frame(last_event_timestamp_);
        if (published) {
            event_counter_ = 0;
            has_pending_events_ = false;
        }
        return published;
    }

    void DisplayMode::timew_histogram(const EventPacket& packet, EventTimestamp time_window){
        if (!initialized || !stop_requested_ || stop_requested_->load() || !frame_) return;

        const auto& events = packet.get_events();
        if (events.empty()) return;

        if (!has_time_window_) {
            time_window_start_ = events[0].timestamp;
            time_window_end_ = time_window_start_ + time_window;
            has_time_window_ = true;
        }

        //going through all events
        for (const auto& ev : events) {
            if (stop_requested_->load()) return;
            const EventTimestamp ev_time = ev.timestamp;
            last_event_timestamp_ = ev_time;
            has_pending_events_ = true;
            frame_->add_event(ev);
            // if ev_time exceeded, time for next frame
            if (ev_time >= time_window_end_) {
                // Move to next window
                time_window_start_ = ev_time;
                if (!frame_->publish_frame(ev_time)) return;
                has_pending_events_ = false;
                time_window_end_ = time_window_start_ + time_window;
            }
        }
    }

    
    void DisplayMode::eventc_histogram(const EventPacket& packet, int event_count){
        if (!initialized || !stop_requested_ || stop_requested_->load() || !frame_ || event_count <= 0) return;

        const auto& events = packet.get_events();
        if (events.empty()) return;

        for (const auto& ev : events) {
            if (stop_requested_->load()) return;
            last_event_timestamp_ = ev.timestamp;
            has_pending_events_ = true;
            frame_->add_event(ev);
            event_counter_++;
            // check if we should start new frame
            if (event_counter_ >= event_count) {
                if (!frame_->publish_frame(ev.timestamp)) return;
                event_counter_ = 0; // ready for updating frame
                has_pending_events_ = false;
            }
        }
    }


    void DisplayMode::make_bi(const EventPacket& packet){
        // TODO: Implement binary frame generation
    }

    void DisplayMode::make_time_surface(const EventPacket& packet){
        // TODO: Implement time surface visualization
    }

    void DisplayMode::finish(){
        if (stop_requested_) {
            stop_requested_->store(true);
        }
        if (frame_) {
            frame_->close();
        }
        frame_.reset();
    }
}
