#pragma once

#include "event_lib/core/event.hpp"
#include "event_lib/core/event_packet.hpp"
#include "event_lib/core/sensor_metadata.hpp"
#include "event_lib/processing/Frame.hpp"
#include "event_lib/processing/FrameQueue.hpp"

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

        // Enqueue a packet using a chosen generation mode; this lets producers
        // push frames while `show()` is running as a consumer.
        void enqueue_packet(const EventPacket& packet, Mode mode = Mode::EventCount,
                    bool colorOn = true, long time_window = 16, int event_count = 10000);

        // Signal that no more frames will be produced. `show()` will exit once
        // the queue is drained.
        void finish();

        //default 60 fps, ts in ms, polarity color changes are on
        void timew_histogram(const EventPacket& packet, bool colorOn, long time_window);
        void timew_histogram(const EventPacket& packet){return timew_histogram(packet, 1, 16);};
        void timew_histogram(const EventPacket& packet, bool colorOn){return timew_histogram(packet, colorOn, 16);};
        void timew_histogram(const EventPacket& packet, long time_window){return timew_histogram(packet, 1, time_window);};


        //default 10k events, oılarity color changes on
        void eventc_histogram(const EventPacket& packet, bool colorOn, int event_count);
        void eventc_histogram(const EventPacket& packet){return eventc_histogram(packet, 1, 10000);};
        void eventc_histogram(const EventPacket& packet, bool colorOn){return eventc_histogram(packet, colorOn, 10000);};
        void eventc_histogram(const EventPacket& packet, int event_count){return eventc_histogram(packet, 1, event_count);};

        
        void make_bi(const EventPacket& packet);

        void make_time_surface(const EventPacket& packet);

        void show(bool colorOn = true);
        // void show_and_save(bool colorOn = true, std::string name);
        // void save(bool colorOn = true, std::string name);

        //TODO: save video

    private:
        const SensorMetadata* metadata_{nullptr};
        FrameQueue frame_queue_;

    };
}