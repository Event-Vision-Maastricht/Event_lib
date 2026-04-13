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
    src/core/event_stream.cpp
)

# Create the library
add_library(event_lib ${EVENT_LIB_SOURCES})

# Set public include directory
target_include_directories(event_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Optional: Enable testing
enable_testing()

# Optional: Add examples (if needed)
# add_subdirectory(examples)