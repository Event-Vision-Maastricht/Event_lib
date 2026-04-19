#include <iostream>
#include <cstdint>
#include <cassert>
#include "../include/event_lib/core/event.hpp"
#include "../include/event_lib/core/event_packet.hpp"

using namespace event_lib;

EventPacket packet;
Event e = create_event(16767,10,10,1);
Event e1 = create_event(16769,11,10,1);
Event e2 = create_event(16771,12,10,1);
Event e3 = create_event(16773,13,10,1);

bool run_test(const std::string& name, bool result) {
    if (result) {
        std::cout << "PASS: " << name << std::endl;
    } else {
        std::cout << "FAIL: " << name << std::endl;
        exit(1);
    }
    return result;
}
//std::vector<Event> events; //event container
bool test_add_event(){
    bool init = packet.is_empty();
    packet.add_event(e);
    packet.add_event(e1);
    packet.add_event(e2);
    packet.add_event(e3);
    bool end = packet.is_empty();
    return init == 1 && end == 0;
}

bool test_get_events(){
    const std::vector<Event>& events = packet.get_events();
    if(events.empty()) return 0;
    return events[0] == e && events[1] == e1 && events[2] == e2 && events[3] == e3;
}

bool test_size(){
    size_t size = packet.size();
    return size == 4;
}

bool test_count_in_range(){
    return packet.count_in_range(0 , 16771) == 3;
}

bool test_get_events_range(){
    std::vector<Event> evs = packet.get_events_in_range(0, 16771);
    return evs.size() == 3 && evs[0] == e && evs[1] == e1 && evs[2] == e2;
}

bool test_clear(){
    packet.clear();
    return packet.is_empty();
}

bool test_is_empty(){
    return packet.is_empty();
}

int main() {
    run_test("test for empty_packet", test_is_empty() == 1);
    run_test("test_add_event", test_add_event() == 1);
    run_test("test for !empty_packet", test_is_empty() == 0);
    run_test("test_get_events", test_get_events() == 1);
    run_test("test_size", test_size()==1);
    run_test("test_count_in_range", test_count_in_range() == 1);
    run_test("test_get_events_range", test_get_events_range() == 1);
    run_test("test_clear", test_clear() == 1);

    return 0;
}