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


bool run_test(const std::string& name, bool result) {
    if (result) {
        std::cout << "PASS: " << name << std::endl;
    } else {
        std::cout << "FAIL: " << name << std::endl;
        std::exit(1);
    }
    return result;
}


bool test_read_show_event_count_visualization_threaded() {
    //const std::string path = "C:/Users/user/Desktop/okul/thesi/data/Davis346blue-2020-02-10T10-57-27+1100-0 ping pong DVS only.aedat";
    //const std::string path = "C:/Users/user/Desktop/okul/thesi/data/Davis346blue-2018-12-30T04-44-31-0800-00000001-0- tobi juggling pasadena shorter.aedat";
    //const std::string path = "C:/Users/user/Desktop/okul/thesi/data/Davis346blue-2018-12-07T13-56-01+0100-00000001-0 disk high BW pr.aedat";
    //const std::string path = "C:/Users/user/Desktop/okul/thesi/data/Davis346mini-2017-11-02T12-05-19+0100-00000001_card_throwing.aedat";
    const std::string path = "C:/Users/user/Desktop/okul/thesi/data/Davis346mini-2017-11-02T15-33-03+0100-00000001-0 nccr girls.aedat";
    //const std::string path = "C:/Users/user/Desktop/okul/thesi/data/a.aedat";

    constexpr std::size_t packet_size = 30000;

    DatasetEventStream stream(path);
    DisplayMode display_mode;
    Window window;

    if (!display_mode.init_metadata(stream.metadata())) return false;
    const auto frame = display_mode.get_frame();
    const auto stop_flag = display_mode.get_stop_flag();
    if (!frame || !stop_flag) return false;
    window.init_window(frame, true, "TestWindowThreaded", stop_flag);

    std::atomic<bool> producer_done{false};
    std::atomic<bool> success{true};
    std::exception_ptr producer_error;

    std::thread producer([&]() {
        try {
            while (stream.has_next() && !stop_flag->load()) {
                EventPacket packet = stream.next_packet(packet_size);
                if (packet.is_empty()) break;
                    //display_mode.eventc_histogram(packet, 5000);
                    //per one ms = 1000 16ms
                    display_mode.timew_histogram(packet, 16000);
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
            break;
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
    run_test("visualize_stream_event_count_histogram", test_read_show_event_count_visualization_threaded());
    return 0;
}
