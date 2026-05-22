#define NOMINMAX
#include "event_lib/processing/DisplayMode.hpp"
#include <algorithm>
#include <iostream>

////////////////LAB2B HAS THE TIME WINDOW HISTOGRAM IMPLEMENTATION
namespace event_lib {

    bool DisplayMode::init_metadata(const SensorMetadata& metadata){
        metadata_ = &metadata;
        frame_ = std::make_shared<Frame>(*metadata_);
        stop_requested_ = std::make_shared<std::atomic<bool>>(false);
        initialized = true;
        return true;
    }

    std::shared_ptr<Frame> DisplayMode::get_frame() const {
        return frame_;
    }

    std::shared_ptr<std::atomic<bool>> DisplayMode::get_stop_flag() const {
        return stop_requested_;
    }

    void DisplayMode::timew_histogram(const EventPacket& packet, long time_window, bool colorOn){
        if (!initialized || !stop_requested_ || stop_requested_->load() || !frame_) return;

        const auto& events = packet.get_events();
        if (events.empty()) return;

        long window_start = events[0].timestamp;
        long window_end = window_start + time_window;

        //going through all events
        for (const auto& ev : events) {
            if (stop_requested_->load()) return;
            long ev_time = ev.timestamp;
            frame_->add_event(ev);
            // if ev_time exceeded, time for next frame
            if (ev_time >= window_end) {
                // Move to next window
                window_start = ev_time;
                //window turns it to false once its shown
                //TODO:if its already true, wait until false(thread version tho)
                frame_->set_dirty(true);
                window_end = window_start + time_window;
            }
        }
        if (frame_->get_total_events() > 0) {
            frame_->set_dirty(true);
        }
    }

    
    void DisplayMode::eventc_histogram(const EventPacket& packet, int event_count, bool colorOn){
        if (!initialized || !stop_requested_ || stop_requested_->load() || !frame_) return;

        const auto& events = packet.get_events();
        if (events.empty()) return;

        int event_counter = 0;
        long frame_ts = events[0].timestamp;

        for (const auto& ev : events) {
            if (stop_requested_->load()) return;
            frame_->add_event(ev);
            event_counter++;
            // check if we should start new frame
            if (event_counter >= event_count) {
                frame_->set_dirty(true);
                frame_ts = ev.timestamp; //new timestamp of the next frame
                event_counter = 0; // ready for updating frame
            }
        }
        if (event_counter > 0) {
            frame_->set_dirty(true);
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
        frame_.reset();
    }
}
