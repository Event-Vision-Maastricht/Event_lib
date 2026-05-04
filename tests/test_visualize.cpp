#include <iostream>
#include <cstdint>
#include <cassert>

//using namespace event_lib;

bool run_test(const std::string& name, bool result) {
    if (result) {
        std::cout << "PASS: " << name << std::endl;
    } else {
        std::cout << "FAIL: " << name << std::endl;
        exit(1);
    }
    return result;
}

int main(){
    run_test("helo", 1);
    return 0;
}