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

uint32_t factorial( uint32_t number ) {
    return number <= 1 ? number : factorial(number-1) * number;
}


int main() {
    run_test("test_create_event", test_create_event() == 1);
    run_test("random",factorial(3) == 6);
    
    std::cout << "########## All tests passed in test_event! ##########" << std::endl;
    return 0;
}