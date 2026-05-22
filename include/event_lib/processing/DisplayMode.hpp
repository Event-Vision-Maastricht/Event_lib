#pragma once

#include "event_lib/core/event.hpp"
#include "event_lib/core/event_packet.hpp"
#include "event_lib/core/sensor_metadata.hpp"
#include "event_lib/processing/Frame.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <cstring>

namespace event_lib {

    class DisplayMode{
    public:
        bool initialized = false;

        bool init_metadata(const SensorMetadata& metadata);
        std::shared_ptr<Frame> get_frame() const;
        std::shared_ptr<std::atomic<bool>> get_stop_flag() const;
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

    private:
        const SensorMetadata* metadata_{nullptr};
        std::shared_ptr<Frame> frame_;
        std::shared_ptr<std::atomic<bool>> stop_requested_;

    };
}