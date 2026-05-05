#include "event_lib/processing/visualize.hpp"
#include <algorithm>
#include <iostream>

////////////////LAB2B HAS THE TIME WINDOW HISTOGRAM IMPLEMENTATION
namespace event_lib {

    bool visualize::init_metadata(int w, int h, std::string d, std::string t, std::string v, std::string et){
        initVals_.width = w;
        initVals_.height = h;
        initVals_.date = d;
        initVals_.time = t;
        initVals_.version = v;
        initVals_.event_type = et;
        initialized = true;
        return true;
    }

    void visualize::timew_histogram(const EventPacket& packet, bool colorOn, long time_window){
        if (!initialized) return;

        Frame frame(initVals_.width, initVals_.height);
        const auto& events = packet.get_events();
        if (events.empty()) return;

        long start_time = events[0].timestamp;
        long window_end = start_time + time_window;
        bool work_on_frame = false;

        for (const auto& ev : events) {
            long ev_time = ev.timestamp;
            frame.add_event(ev);
            work_on_frame = true;

            // if ev_time exceeded, time for next frame
            if (ev_time >= window_end) {
                frame.finalize_frame(start_time);
                start_time = ev_time;

                // Move to next window
                work_on_frame = false;
                window_end = start_time + time_window;
            }
        }
        if (work_on_frame) {
            frame.finalize_frame(start_time);
        }
        show(frame.extract_packet());
    }

    void visualize::eventc_histogram(const EventPacket& packet, bool colorOn, int event_count){
        if (!initialized) return;

        Frame frame(initVals_.width, initVals_.height);
        const auto& events = packet.get_events();

        if (events.empty()) return;

        int event_counter = 0;
        long frame_ts = events[0].timestamp;

        for (const auto& ev : events) {
            frame.add_event(ev);
            event_counter++;

            // check if we should start new frame
            if (event_counter >= event_count) {
                frame.finalize_frame(frame_ts);
                frame_ts = ev.timestamp;

                // Reset for next batch
                frame.reset_frame();
                event_counter = 0;
            }
        }
        // Save final frame if there are remaining events
        if (event_counter > 0) frame.finalize_frame(frame_ts);

        show(frame.extract_packet());

    }

    void visualize::make_bi(const EventPacket& packet){
        // TODO: Implement binary frame generation
    }

    void visualize::make_time_surface(const EventPacket& packet){
        // TODO: Implement time surface visualization
    }


    void visualize::show(FramePacket fpack){
        (void)fpack;
        // TODO: render or persist the frame packet
    }
}