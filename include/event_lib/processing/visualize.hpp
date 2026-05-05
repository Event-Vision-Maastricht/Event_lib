#pragma once

#include "../core/event_packet.hpp"
#include "event_lib/processing/Frame.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <cstring>

namespace event_lib {

    struct SensorMetadata{
        int width;
        int height;
        std::string date;/////     Recording Date, format: YYYY-MM-DD HH:MM:SS
        std::string time;
        std::string version;// Format version
        std::string event_type; //Type of event: CD/2d/ExtTrig
    };

    class visualize{
    public:
        bool initialized = false;

        bool init_metadata(int w, int h, std::string d, std::string t, std::string v, std::string et);

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

        //TODO: save video

    private:
        SensorMetadata initVals_;
        void show(FramePacket fpack);

    };
}