#include "event_lib/processing/DisplayMode.hpp"
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
// EventPacket packet;


bool run_test(const std::string& name, bool result) {
    if (result) {
        std::cout << "PASS: " << name << std::endl;
    } else {
        std::cout << "FAIL: " << name << std::endl;
        std::exit(1);
    }
    return result;
}

int main() {
    //run_test("visualize_stream_event_window_histogram", test_read_show_eventW());
    //run_test("visualize_stream_time_window_histogram", test_read_show_timeW());
    return 0;
}