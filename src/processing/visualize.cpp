#include "event_lib/processing/visualize.hpp"
//#include "event_lib/core/event.hpp"

////////////////LAB2B HAS THE TIME WINDOW HISTOGRAM IMPLEMENTATION
namespace event_lib {
    //    std::vector<int> make_histogram(const EventPacket& packet, bool colorOn=1){}
        void timew_histogram(const EventPacket& packet, bool colorOn, long time_window){
            //start time and current time
            long start_time = packet.get_packet_start_time();
            long current_time = start_time;

            //TODO: get width and height info and create the frame.
            //int[][] frame;
            //create frame
            return;
        }
        void eventc_histogram(const EventPacket& packet, bool colorOn, int event_count){
            return;
        }



        /////////////////////////////TODO///////////////////////////////
        void make_bi(const EventPacket& packet){
            return;
        }
        void make_time_surface(const EventPacket& packet){
            return;
        }
}