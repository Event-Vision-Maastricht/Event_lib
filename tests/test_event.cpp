#include <iostream>
#include <cstdint>
#include <cassert>
#include "../include/event_lib/core/event.hpp"
using namespace event_lib;

bool run_test(const std::string& name, bool result) {
    if (result) {
        std::cout << "PASS: " << name << std::endl;
    } else {
        std::cout << "FAIL: " << name << std::endl;
        exit(1);
    }
    return result;
}

bool test_create_event() {
    Event e = create_event(1.6767, 10,10,1);
    Event e1 = create_event(1.6767,10,10,1);
    if(e == Event() || e != e1){
        std::cout << "CREATE EVENT FAILED";
        return 0;
    }
    return true;
}

bool test_valid_event() {
    Event e = create_event(1.6767, 10,10,1);
    return is_valid_event(e,11,11) == 1 && is_valid_event(e,9,9) ==0;
}

bool test_get_ts() {
    Event e = create_event(1.6767, 10,10,1);
    return get_timestamp(e) == (long long) 1.6767;
}

bool test_get_coordinates() {
    Event e = create_event(1.6767, 10, 10, 1);
    auto coords = get_coordinates(e);
    return coords[0] == 10 && coords[1] == 10;
}

bool test_get_polarity() {
    Event e = create_event(1.6767, 10,10,1);
    return (double) get_polarity(e) == 1;
}


int main() {
    run_test("test_create_event", test_create_event() == 1);
    run_test("test_valid_event", test_valid_event() == 1);
    run_test("test_get_ts", test_get_ts()==1);
    run_test("test_get_coordinates", test_get_coordinates()==1);
    run_test("test_get_polarity", test_get_polarity()==1);
    return 0;
}