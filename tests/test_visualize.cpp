#include "event_lib/processing/DisplayMode.hpp"
#include "event_lib/processing/Window.hpp"
#include "event_lib/io/parser/DatParser.hpp"
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
    constexpr std::size_t packet_size = 10000;

    DatParser parser;
    try {
        parser.open(dat_path);
    } catch (const std::exception& e) {
        std::cerr << "Could not open dat file: " << e.what() << std::endl;
        return false;
    }

    if (!parser.has_more()) {
        std::cerr << "Parser has no events after open()." << std::endl;
        parser.close();
        return false;
    }

    const SensorMetadata& metadata = parser.header();
    DisplayMode display_mode;
    if (!display_mode.init_metadata(metadata)) {
        std::cerr << "Could not initialize DisplayMode metadata." << std::endl;
        parser.close();
        return false;
    }

    Window window;
    window.init_window(display_mode.get_frame(), true, "Event Count Visualization", display_mode.get_stop_flag());
    if (display_mode.get_frame() == nullptr) {
        std::cerr << "Window could not bind to the display frame." << std::endl;
        display_mode.finish();
        parser.close();
        return false;
    }

    bool rendered_any_packet = false;
    while (parser.has_more()) {
        EventPacket packet;
        try {
            packet = parser.read_packet(packet_size);
        } catch (const std::exception& e) {
            std::cerr << "Could not read packet: " << e.what() << std::endl;
            window.finish();
            display_mode.finish();
            parser.close();
            return false;
        }

        if (packet.is_empty()) {
            continue;
        }

        rendered_any_packet = true;
        display_mode.eventc_histogram(packet, static_cast<int>(packet_size), true);
        window.show_frame(packet.get_events().back().timestamp);
        display_mode.get_frame()->reset_frame();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (!rendered_any_packet) {
        std::cerr << "No packets were rendered from the DAT file." << std::endl;
        window.finish();
        display_mode.finish();
        parser.close();
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    window.show_frame(display_mode.get_frame()->get_last_update());

    window.finish();
    display_mode.finish();
    parser.close();
    return true;
}

int main() {
    run_test("visualize_stream_event_count_histogram", test_read_show_event_count_visualization());
    return 0;
}