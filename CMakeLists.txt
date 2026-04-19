cmake_minimum_required(VERSION 3.15)
project(event_lib VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Collect all source files
set(EVENT_LIB_SOURCES
    src/core/event.cpp
    src/core/event_packet.cpp
    # src/io/camera/CameraEventStream.cpp
    src/io/dataset/DatasetEventStream.cpp
)

# Create the library
add_library(event_lib ${EVENT_LIB_SOURCES})

# Set public include directory
target_include_directories(event_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Test
enable_testing()
    # test_event
add_executable(test_event tests/test_event.cpp)
target_link_libraries(test_event PRIVATE event_lib)
target_include_directories(test_event PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    #test_build_include_run
add_executable(test_build_include_run tests/test_build_include_run.cpp)
target_link_libraries(test_build_include_run PRIVATE event_lib)

    # test_event_packet
add_executable(test_event_packet tests/test_event_packet.cpp)
target_link_libraries(test_event_packet PRIVATE event_lib)
target_include_directories(test_event_packet PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


add_test(NAME EventTests COMMAND test_event)
add_test(NAME BuildIncludeRunTests COMMAND test_build_include_run)
add_test(NAME EventPacketTests COMMAND test_event_packet)


# Optional: Add examples (if needed)
# add_subdirectory(examples)