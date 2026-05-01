#include <iostream>
#include <cstdint>
#include <cassert>
#include "../include/event_lib/io/parser/DatParser.hpp"
#include <exception>

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

//             % Data file containing CD events.
//             % Version 2
//             % Date 2020-09-14 16:03:08
//             % Height 480
//             % Width 640
bool test_header_reading(){
    DatParser parser;
    try {
        parser.open("C:/Users/user/Desktop/okul/thesi/data/spinner.dat");
    } catch (...) {
        std::cerr << "Could not open dat file" << std::endl;
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
    parser.close();
    return true;
}

bool test_open_file(){
    return 1;
}

int main() {
    run_test("header testing", test_header_reading());
    run_test("open file", test_open_file());

    return 0;
}