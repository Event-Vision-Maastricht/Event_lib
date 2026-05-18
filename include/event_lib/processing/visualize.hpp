#pragma once

#include "event_lib/core/event.hpp"
#include "event_lib/core/event_packet.hpp"
#include "event_lib/core/sensor_metadata.hpp"
#include "event_lib/processing/Frame.hpp"

#include <opencv2/core.hpp>
#include <atomic>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>

namespace event_lib {

    class visualize{
    public:
        bool initialized = false;

        enum class Mode {
            TimeWindow,
            EventCount,
            Binary,
            TimeSurface
        };

        bool init_metadata(const SensorMetadata& metadata);

        void init_show(const std::string& window_name = "Event Lib", bool colorOn = true);

        // Enqueue a packet using a chosen generation mode; this lets producers
        // push frames while `show()` is running as a consumer.
        void enqueue_packet(const EventPacket& packet, Mode mode = Mode::EventCount,
                    bool colorOn = true, long time_window = 16, int event_count = 10000);

        // Signal that no more frames will be produced. `show()` will exit once
        // the queue is drained.
        void finish();

        //default 60 fps, ts in ms, polarity color changes are on
        void timew_histogram(const EventPacket& packet){return timew_histogram(packet, 16, true);};
        void timew_histogram(const EventPacket& packet, int time_window){return timew_histogram(packet, static_cast<long>(time_window), true);};
        void timew_histogram(const EventPacket& packet, long time_window){return timew_histogram(packet, time_window, true);};
        void timew_histogram(const EventPacket& packet, long time_window, bool colorOn);


        //default 10k events, oılarity color changes on
        void eventc_histogram(const EventPacket& packet){return eventc_histogram(packet, 10000);};
        void eventc_histogram(const EventPacket& packet, int event_count){return eventc_histogram(packet, event_count, true);};
        void eventc_histogram(const EventPacket& packet, int event_count, bool colorOn);

        
        void make_bi(const EventPacket& packet);

        void make_time_surface(const EventPacket& packet);

        void show_frame(long frame_time);
        void show(bool colorOn = true);
        // void show_and_save(bool colorOn = true, std::string name);
        // void save(bool colorOn = true, std::string name);

        //TODO: save video

        // Check if stop has been requested (e.g., user closed window)
        bool is_stop_requested() const { return stop_requested_.load(); }

    private:
        const SensorMetadata* metadata_{nullptr};
        std::unique_ptr<Frame> frame_;
        std::atomic<bool> stop_requested_{false};
        bool window_initialized_{false};
        bool color_on_{true};
        cv::Mat image_;
        
        std::string window_name_ = "Event Lib";
//        cv::namedWindow(window_name, cv::WINDOW_NORMAL);



    };
}