#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include "event_lib/processing/DisplayMode.hpp"
#include <algorithm>
//#include <cmath>
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
        // has_time_surface_ = false;
        // time_surface_on_.assign(metadata.height, std::vector<EventTimestamp>(metadata.width, 0));
        // time_surface_off_.assign(metadata.height, std::vector<EventTimestamp>(metadata.width, 0));
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

/**

    //assuming events are sorted in ascending timestamp
    void DisplayMode::make_time_surface(const EventPacket& packet){
        if (!initialized || !metadata_ || !stop_requested_ || stop_requested_->load() || !frame_) return;
        const auto& events = packet.get_events();
        if (events.empty()) return;
        if (!has_time_surface_) {
            time_surface_on_.assign(metadata_->height, std::vector<EventTimestamp>(metadata_->width, 0));
            time_surface_off_.assign(metadata_->height, std::vector<EventTimestamp>(metadata_->width, 0));
            has_time_surface_ = true;
        }

        constexpr int neighborhood_radius = 2;      // 5x5 local time surface patch.
        constexpr double tau_ms = 30.0;             // Temporal decay constant for millisecond timestamps.
        constexpr int max_intensity = 255;

        FrameStr time_surface_frame;
        time_surface_frame.on_events.assign(metadata_->height, std::vector<int>(metadata_->width, 0));
        time_surface_frame.off_events.assign(metadata_->height, std::vector<int>(metadata_->width, 0));

        for (const auto& ev : events) {
            if (stop_requested_->load()) return;

            auto& timestamp_map = ev.polarity ? time_surface_on_ : time_surface_off_;
            auto& output_map = ev.polarity ? time_surface_frame.on_events : time_surface_frame.off_events;

            timestamp_map[ev.y][ev.x] = ev.timestamp;
            last_event_timestamp_ = ev.timestamp;
            has_pending_events_ = true;

            const int min_x = std::max(0, ev.x - neighborhood_radius);
            const int max_x = std::min(metadata_->width - 1, ev.x + neighborhood_radius);
            const int min_y = std::max(0, ev.y - neighborhood_radius);
            const int max_y = std::min(metadata_->height - 1, ev.y + neighborhood_radius);

            for (int y = min_y; y <= max_y; ++y) {
                for (int x = min_x; x <= max_x; ++x) {
                    const EventTimestamp neighbor_timestamp = timestamp_map[y][x];
                    if (neighbor_timestamp <= 0 || neighbor_timestamp > ev.timestamp) continue;

                    const double age_ms = static_cast<double>(ev.timestamp - neighbor_timestamp);
                    const int intensity = static_cast<int>(std::round(max_intensity * std::exp(-age_ms / tau_ms)));
                    output_map[y][x] = std::max(output_map[y][x], intensity);
                }
            }
        }

        time_surface_frame.timestamp = last_event_timestamp_;
        if (has_pending_events_ && frame_->publish_frame(time_surface_frame)) {
            has_pending_events_ = false;
        }
    }
    
 */

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
