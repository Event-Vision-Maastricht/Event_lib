#include <iostream>
#include <cstdint>
#include <cassert>
#include "../include/event_lib/io/parser/AedatParser.hpp"
#include "../include/event_lib/core/event_packet.hpp"
#include "../include/event_lib/core/event.hpp"

#include <exception>

using namespace event_lib;
    AedatParser parser;
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
     * #!AER-DAT2.0             /////
     * # This is a raw AE data file - do not edit
     * # Data format is int32 address, int32 timestamp (8 bytes total), repeated for each event
     * # Timestamps tick: 1 us
     * # Creation date: Mon Feb 10 10:57:27 AEDT 2020           ////////
     * # Creation time: System.currentTimeMillis() 1581292647203
     * # User name: tobid
     * # Hostname: LAPTOP-SS5VU6HO
     * # AEChip: eu.seebetter.ini.chips.davis.Davis346blue              /////////
     * #Start of Preferences for this AEChip (search for "End of Preferences" to find end of this block)
     */
bool test_header_reading(){
    try {
        parser.open("C:/Users/user/Desktop/okul/thesi/data/Davis346blue-2020-02-10T10-57-27+1100-0 ping pong DVS only.aedat");
        
    } catch (const std::exception& e) {
        std::cerr << "Could not open raw file: " << e.what() << std::endl;
        return false;
    }
    auto hdr = parser.header();
    std::string version = hdr.version.value_or("");
        std::cout << "Header Processed:\n"
        << "  version: " << version << " (expected: 2.0)\n"
        << "  date: " << hdr.date << " (expected: 2020-02-10)\n"
        << "  time: " << hdr.time << " (expected: 10:57:27)\n"
        << "  height: " << hdr.height << " (expected: 260)\n"
        << "  width: " << hdr.width << " (expected: 346)" << std::endl;
    // Check if header matches expected values
    if(version != "2.0" ||hdr.date!="2020-02-10" || hdr.time!="10:57:27" ||hdr.height != 260 || hdr.width != 346){
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