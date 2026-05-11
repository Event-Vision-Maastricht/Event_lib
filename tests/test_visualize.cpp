#include "event_lib/processing/visualize.hpp"
#include "event_lib/core/sensor_metadata.hpp"
#include "event_lib/io/stream/DatasetEventStream.hpp"
#include "event_lib/core/sensor_metadata.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace fs = std::filesystem;

using namespace event_lib;
EventPacket packet;
visualize vis;

bool run_test(const std::string& name, bool result) {
    if (result) {
        std::cout << "PASS: " << name << std::endl;
    } else {
        std::cout << "FAIL: " << name << std::endl;
        std::exit(1);
    }
    return result;
}

// bool test_init_metadata(){
//      bool visualazie_init = vis.init_metadata(streamer.get_width(),streamer.get_height(),
//      streamer.get_date(), streamer.get_init_recording_time(), streamer.get_version(), streamer.get_event_type());
//     if(!visualazie_init){
//         std::cerr << "Could not initialize metadata for visualizer. " << std::endl;
//         return false;
//     }

    

//         if(hdr.event_type != "CD" || hdr.version != "2" ||hdr.date!="2020-09-14" || hdr.time!="16:03:08" ||hdr.height != 480 || hdr.width != 640){
//         std::cout << "Header mismatch:\n";
//         return false;
//     }

//     return true;
// }

bool test_read_show_eventW(){
    DatasetEventStream streamer("C:/Users/user/Desktop/okul/thesi/data/spinner.dat");
    packet = streamer.next_packet(10000);
    const SensorMetadata& metadata = streamer.metadata();

    bool visualazie_init = vis.init_metadata(metadata);
    if(!visualazie_init){
        std::cerr << "Could not initialize metadata for visualizer. " << std::endl;
        return false;
    }

    std::exception_ptr producer_error = nullptr;
    std::thread producer([&]() {
        try {
            while (true) {
                vis.eventc_histogram(packet, 2500);
                if (vis.is_stop_requested()) break;  // Exit immediately if user closed window
                if (!streamer.has_next()) break;
                packet = streamer.next_packet(10000);
            }

            vis.finish();
        } catch (...) {
            producer_error = std::current_exception();
            vis.finish();
        }
    });
    
    try {
        vis.show(true);
    } catch (const std::exception& e) {
        std::cerr << "Visualization stream error: " << e.what() << std::endl;
        vis.finish();
        if (producer.joinable()) {
            producer.join();
        }
        return false;
    }

    if (producer.joinable()) {
        producer.join();
    }

    if (producer_error) {
        try {
            std::rethrow_exception(producer_error);
        } catch (const std::exception& e) {
            std::cerr << "Producer error: " << e.what() << std::endl;
        }
        return false;
    }

    return true;
}

bool test_read_show_timeW(){
    DatasetEventStream streamer("C:/Users/user/Desktop/okul/thesi/data/spinner.dat");
    packet = streamer.next_packet(10000);
    const SensorMetadata& metadata = streamer.metadata();

    bool visualazie_init = vis.init_metadata(metadata);

    std::exception_ptr producer_error = nullptr;
    std::thread producer([&]() {
        try {
            while (true) {
                //std::cerr << "DEBUG: reading packet from stream" << std::endl;
                vis.timew_histogram(packet, 60);
                if (vis.is_stop_requested()) break;
                if (!streamer.has_next()) {
                    break;
                }
                packet = streamer.next_packet(10000);
            }

            vis.finish();
        } catch (...) {
            producer_error = std::current_exception();
            vis.finish();
        }
    });

    try {
        vis.show(true);
    } catch (const std::exception& e) {
        std::cerr << "Visualization stream error: " << e.what() << std::endl;
        vis.finish();
        if (producer.joinable()) {
            producer.join();
        }
        return false;
    }

    if (producer.joinable()) {
        producer.join();
    }

    return true;
}


int main() {
    //run_test("visualize_stream_event_window_histogram", test_read_show_eventW());
    run_test("visualize_stream_time_window_histogram", test_read_show_timeW());
    return 0;
}