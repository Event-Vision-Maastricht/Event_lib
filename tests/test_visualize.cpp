#include "event_lib/processing/DisplayMode.hpp"
#include "event_lib/processing/Window.hpp"
#include "event_lib/io/stream/DatasetEventStream.hpp"
#include "event_lib/core/event_packet.hpp"
#include "event_lib/core/sensor_metadata.hpp"

#include <cstdlib>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

using namespace event_lib;

bool should_hold_visual_window() {
    const char* value = std::getenv("EVENT_LIB_VISUAL_HOLD");
    return value == nullptr || std::string(value) != "0";
}

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
///TODO: right now histograms do not stop/return when frame is updated and ready. so when window can visualize it, the whole packet is already processed.
// thets why right now the max event or max time does not work for anything. possible slutions:
//1- work in threads so window and display mode are boh constantly called simultaneously.
//2- displaymode returns after completing each frame update, pointer to the specific event on package to be able to continue.
    try{
        while (stream.has_next() && stop_flag && !stop_flag->load()) {
            EventPacket packet = stream.next_packet(packet_size);
            if (packet.is_empty()) break;

//            display_mode.eventc_histogram(packet, static_cast<int>(packet.size()));
            display_mode.eventc_histogram(packet,10000);
            //display_mode.timew_histogram(packet, 120);
            window.show_frame();
        }
        if (display_mode.flush_pending_frame()) {
            window.show_frame();
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

bool test_read_show_event_count_visualization_threaded() {
    const std::string dat_path = "C:/Users/user/Desktop/okul/thesi/data/spinner.dat";
    constexpr std::size_t packet_size = 30000;

    DatasetEventStream stream(dat_path);
    DisplayMode display_mode;
    Window window;

    //initialize metadata for display mode
    if (!display_mode.init_metadata(stream.metadata())) return false;

    //get frame pointer and stop flag
    const auto frame = display_mode.get_frame();
    const auto stop_flag = display_mode.get_stop_flag();
    if (!frame || !stop_flag) return false;

    //initialize window by frame pointer, color on, window name and stop flag pointer
    window.init_window(frame, true, "TestWindowThreaded", stop_flag);

    std::atomic<bool> producer_done{false};
    std::atomic<bool> success{true};
    std::exception_ptr producer_error;

    std::thread producer([&]() {
        try {
            while (stream.has_next() && !stop_flag->load()) {
                EventPacket packet = stream.next_packet(packet_size);
                if (packet.is_empty()) break;
                    //display_mode.eventc_histogram(packet, 30000);
                    display_mode.timew_histogram(packet, 1200);

            }
            display_mode.flush_pending_frame();
        } catch (...) {
            success.store(false);
            producer_error = std::current_exception();
            stop_flag->store(true);
        }

        producer_done.store(true);
    });

    while (!stop_flag->load()) {
        if (stop_flag->load()) break;

        const bool frame_ready = frame->wait_for_published_frame(std::chrono::milliseconds(16));
        if (frame_ready && frame->is_dirty()) {
            window.show_frame();
        } else if (producer_done.load()) {
            if (!should_hold_visual_window()) break;
            window.show_frame();
        } else {
            window.show_frame();
        }
    }
    producer.join();

    window.finish();

    if (producer_error) {
        try {
            std::rethrow_exception(producer_error);
        } catch (const std::exception& e) {
            std::cerr << "ERROR in stream demo thread: " << e.what() << std::endl;
        }
        return false;
    }

    display_mode.finish();
    stream.close();

    return success.load();
}

int main() {
    //run_test("visualize_stream_event_count_histogram", test_read_show_event_count_visualization());
    run_test("visualize_stream_event_count_histogram threaded.", test_read_show_event_count_visualization_threaded());
    return 0;
}
