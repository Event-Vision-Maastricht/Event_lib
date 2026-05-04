#pragma once

#include "../core/event_packet.hpp"
#include"event_lib/processing/visualize.hpp"
#include <vector>

namespace event_lib {

    std::vector<int> make_histogram(const EventPacket& packet, bool colorOn=1);
    void timew_histogram(const EventPacket& packet, bool colorOn=1){

    }
    void eventc_histogram(const EventPacket& packet, bool colorOn=1){

    }
    void make_bi(const EventPacket& packet, bool colorOn=1){

    }
    void make_time_surface(const EventPacket& packet, bool colorOn=1){
        
    }



}