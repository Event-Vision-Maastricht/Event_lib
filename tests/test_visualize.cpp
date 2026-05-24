#include "event_lib/processing/DisplayMode.hpp"
#include "event_lib/processing/Window.hpp"
#include "event_lib/io/stream/DatasetEventStream.hpp"
#include "event_lib/core/event_packet.hpp"
#include "event_lib/core/sensor_metadata.hpp"

#include <cstdlib>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

using namespace event_lib;


bool run_test(const std::string& name, bool result) {
    if (result) {
        std::cout << "PASS: " << name << std::endl;
    } else {
        std::cout << "FAIL: " << name << std::endl;
        std::exit(1);
    }
    return result;
}

bool test_read_show_event_count_visualization() {
    const std::string dat_path = "C:/Users/user/Desktop/okul/thesi/data/spinner.dat";
    //const std::string dat_path = "C:/Users/user/Desktop/okul/thesi/data/test_start_after_0.dat";
    //const std::string dat_path = "C:/Users/user/Desktop/okul/thesi/data/events_big_time_gap_repeated_ts.dat";
    //const std::string dat_path = "C:/Users/user/Desktop/okul/thesi/data/Prophesee_Dataset_n_cars/n-cars_test/cars/obj_000121_td.dat";

    
    constexpr std::size_t packet_size = 10000;

    DatasetEventStream stream(dat_path);
    DisplayMode display_mode;
    Window window;

    if (!display_mode.init_metadata(stream.metadata())) return false;
    window.init_window(display_mode.get_frame(), true, "TestWindow", display_mode.get_stop_flag());
    const auto stop_flag = display_mode.get_stop_flag();

    try{
        while (stream.has_next() && stop_flag && !stop_flag->load()) {
            EventPacket packet = stream.next_packet(packet_size);
            if (packet.is_empty()) break;

            //display_mode.eventc_histogram(packet, static_cast<int>(packet.size()), true);
            display_mode.timew_histogram(packet, 120, true);

            window.show_frame();

            //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        if (stop_flag) {
            stop_flag->store(true);
        }

        //window.finish();
        display_mode.finish();
        stream.close();
    }catch(const std::exception& e){
        std::cerr << "ERROR in stream demo: " << e.what() << std::endl;
        if (stop_flag) {
            stop_flag->store(true);
        }
        return false;
    }

    return true;

}

int main() {
    run_test("visualize_stream_event_count_histogram", test_read_show_event_count_visualization());
    return 0;
}