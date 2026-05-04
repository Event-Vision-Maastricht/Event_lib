#include <iostream>
#include <cstdint>
#include <cassert>
#include "../include/event_lib/io/parser/DatParser.hpp"
#include "../include/event_lib/core/event_packet.hpp"
#include "../include/event_lib/core/event.hpp"

#include <exception>

using namespace event_lib;
    DatParser parser;
    EventPacket packet;

bool run_test(const std::string& name, bool result) {
    if (result) {
        std::cout << "PASS: " << name << std::endl;
    } else {
        std::cout << "FAIL: " << name << std::endl;
        exit(1);
    }
    return result;
}

//             % Data file containing CD events.
//             % Version 2
//             % Date 2020-09-14 16:03:08
//             % Height 480
//             % Width 640
bool test_header_reading(){
    try {
        parser.open("C:/Users/user/Desktop/okul/thesi/data/spinner.dat");
    } catch (const std::exception& e) {
        std::cerr << "Could not open dat file: " << e.what() << std::endl;
        return false;
    }
    auto hdr = parser.header();
        std::cout << "Header Processed:\n"
        << "  type: " << hdr.event_type << " (expected: CD)\n"
        << "  version: " << hdr.version << " (expected: 2)\n"
        << "  date: " << hdr.date << "(expected: 2020-09-14)\n"
        << "  time: " << hdr.time << "(expected: 16:03:08)\n"
        << "  height: " << hdr.height << " (expected: 480)\n"
        << "  width: " << hdr.width << " (expected: 640)" << std::endl;
    // Check if header matches expected values
    if(hdr.event_type != "CD" || hdr.version != "2" ||hdr.date!="2020-09-14" || hdr.time!="16:03:08" ||hdr.height != 480 || hdr.width != 640){
        std::cout << "Header mismatch:\n";
        return false;
    }
    //parser.close();
    return true;
}

bool test_read_event(int amount){
    try{
        packet = parser.read_packet(amount);
    }catch(const std::exception& e){
        std::cerr << "Something went wrong while reading: " << e.what() << std::endl;
        return false;
    }
    if(packet.size() !=amount) return false;

    std::vector<Event> e = packet.get_events();

    for(int i =0; i<amount; i++){
        Event ev = e[i];
        // std::cout << "Event Number:"<< i+1<<"\n"
        //     << "  Timestamp: " << ev.timestamp << "\n"
        //     << "  Polarity: " << ev.polarity << " \n"
        //     << "  X axis: " << ev.x << "\n"
        //     << "  Y axis: " << ev.y << std::endl;

        std::cout <<"  Timestamp: " << ev.timestamp << "  Polarity: " << ev.polarity << "  X axis: " << ev.x << "  Y axis: " << ev.y << std::endl;
    }
    return true;
}

int main() {
    run_test("header testing", test_header_reading());
    run_test("read events", test_read_event(100));

    return 0;
}