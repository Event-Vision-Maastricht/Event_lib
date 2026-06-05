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
        bool flush_pending_frame();
        void finish();

        //default 60 fps, ts in ms, polarity color changes are on
        void timew_histogram(const EventPacket& packet){return timew_histogram(packet, 16);};
        void timew_histogram(const EventPacket& packet, EventTimestamp time_window);

        //default 10k events, oılarity color changes on
        void eventc_histogram(const EventPacket& packet){return eventc_histogram(packet, 10000);};
        void eventc_histogram(const EventPacket& packet, int event_count);

        /**
         * method to make taken from:
         * Speed Invariant Time Surface for Learning to Detect Corner Points with Event-Based Cameras
         * Jacques Manderscheid1, Amos Sironi1, Nicolas Bourdis1, Davide Migliore1and Vincent Lepetit21Prophesee,
         * Paris, France, 2University of Bordeaux, Bordeaux, France
         * and:
         * Efficient spatio-temporal feature clustering for large event-based datasets
         */
        //void make_time_surface(const EventPacket& packet);

    private:
        const SensorMetadata* metadata_{nullptr};
        std::shared_ptr<Frame> frame_;
        std::shared_ptr<std::atomic<bool>> stop_requested_;
        int event_counter_{0};
        EventTimestamp last_event_timestamp_{0};
        EventTimestamp time_window_start_{0};
        EventTimestamp time_window_end_{0};
        bool has_time_window_{false};
        bool has_pending_events_{false};
        // bool has_time_surface_{false};
        // std::vector<std::vector<EventTimestamp>> time_surface_on_;
        // std::vector<std::vector<EventTimestamp>> time_surface_off_;
    };
}
