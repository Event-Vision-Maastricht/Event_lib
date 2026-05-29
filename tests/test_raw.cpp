#include <iostream>
#include <cstdint>
#include <cassert>
#include "../include/event_lib/io/parser/RawParser.hpp"
#include "../include/event_lib/core/event_packet.hpp"
#include "../include/event_lib/core/event.hpp"

#include <exception>

using namespace event_lib;
    RawParser parser;
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

    /**
     * % Date 2020-09-14 09:03:25
     * % firmware_version 2.0.2
     * % integrator_name Prophesee
     * % plugin_name hal_plugin_gen3_fx3
     * % serial_number 30384338
     * % system_ID 21
     * % evt 2.0
     */
bool test_header_reading(){
    try {
        parser.open("C:/Users/user/Desktop/okul/thesi/data/spinner.raw");
        
    } catch (const std::exception& e) {
        std::cerr << "Could not open raw file: " << e.what() << std::endl;
        return false;
    }
    auto hdr = parser.header();
    std::string event_type = hdr.get_extra_or("evt");
    std::string firmware_version = hdr.get_extra_or("firmware_version");
    std::string plugin_name = hdr.get_extra_or("plugin_name");
    std::string serial_number = hdr.get_extra_or("serial_number");
    std::string system_id = hdr.get_extra_or("system_id");
    std::string integrator_name = hdr.get_extra_or("integrator_name");

    std::cout << "Header Processed:\n"
        << "  date: " << hdr.date << "(expected: 2020-09-14)\n"
        << "  time: " << hdr.time << "(expected: 09:03:25)\n"
        << "  height: " << hdr.height << " (expected: 480)\n"
        << "  width: " << hdr.width << " (expected: 640)\n"
        << "  event type: " << event_type << " (expected: 2.0)\n"
        << "  firmware version: " << firmware_version << " (expected: 2.0.2)\n"
        << "  plugin name: " << plugin_name << " (expected: hal_plugin_gen3_fx3)\n"
        << "  serial number: " << serial_number << " (expected: 30384338)\n"
        << "  system id: " << system_id << " (expected: 21)\n"
        << "  integrator name: " << integrator_name << " (expected: Prophesee)\n"
        << std::endl;
    // Check if header matches expected values
    if(event_type != "2.0" || firmware_version != "2.0.2" ||hdr.date!="2020-09-14" || hdr.time!="09:03:25" ||hdr.height != 480
        || hdr.width != 640 || plugin_name != "hal_plugin_gen3_fx3" || serial_number !="30384338" || system_id != "21" || integrator_name != "Prophesee"){
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
        std::cout <<"  Timestamp: " << ev.timestamp << "  Polarity: " << ev.polarity << "  X axis: " << ev.x << "  Y axis: " << ev.y << std::endl;
    }
    return true;
}

int main() {
    run_test("header testing", test_header_reading());
    run_test("read events", test_read_event(100));

    return 0;
}