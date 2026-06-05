cmake_minimum_required(VERSION 3.15)
project(event_lib_scalability_benchmarks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EVENT_LIB_SOURCE_DIR "" CACHE PATH "Optional path to the Event_lib source tree")

if (EVENT_LIB_SOURCE_DIR)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    add_subdirectory("${EVENT_LIB_SOURCE_DIR}" event_lib_from_source)
else()
    find_package(event_lib CONFIG REQUIRED)
endif()

add_executable(parser_benchmark parser_benchmark.cpp)
target_link_libraries(parser_benchmark PRIVATE event_lib::event_lib)

add_executable(representation_benchmark representation_benchmark.cpp)
target_link_libraries(representation_benchmark PRIVATE event_lib::event_lib)
