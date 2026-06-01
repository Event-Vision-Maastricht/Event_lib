cmake_minimum_required(VERSION 3.15)
project(event_lib VERSION 1.0.0 LANGUAGES CXX)

find_package(OpenCV REQUIRED COMPONENTS core highgui imgcodecs videoio)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Collect all source files
set(EVENT_LIB_SOURCES
    src/core/event.cpp
    src/core/event_packet.cpp
    src/io/parser/EventParserFactory.cpp
    src/io/parser/AedatParser.cpp
    src/io/parser/DatParser.cpp
    src/io/parser/RawParser.cpp
    src/io/stream/DatasetEventStream.cpp
    src/processing/DisplayMode.cpp
    src/processing/Frame.cpp
    src/processing/Window.cpp
)

# Create the library
add_library(event_lib ${EVENT_LIB_SOURCES})

target_link_libraries(event_lib PUBLIC ${OpenCV_LIBS})

# Conda OpenCV on Windows is often release-only; this avoids Debug symbol
# name mismatch (cv::debug_build_guard::...) when building with MSVC Debug.
if (MSVC)
    target_compile_definitions(event_lib PUBLIC CV_IGNORE_DEBUG_BUILD_GUARD)
endif()

# Set public include directory
target_include_directories(event_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(event_lib PUBLIC ${OpenCV_INCLUDE_DIRS})

################################### Test
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

    # test_datH
add_executable(test_datH tests/test_datH.cpp)
target_link_libraries(test_datH PRIVATE event_lib)
target_include_directories(test_datH PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    # test_raw
add_executable(test_raw tests/test_raw.cpp)
target_link_libraries(test_raw PRIVATE event_lib)
target_include_directories(test_raw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    #test_aedat
add_executable(test_aedat tests/test_aedat.cpp)
target_link_libraries(test_aedat PRIVATE event_lib)
target_include_directories(test_aedat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    #test_visualize
add_executable(test_visualize tests/test_visualize.cpp)
target_link_libraries(test_visualize PRIVATE event_lib)
target_include_directories(test_visualize PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    #test_visualize_raw
add_executable(test_visualize_raw tests/test_visualize_raw.cpp)
target_link_libraries(test_visualize_raw PRIVATE event_lib)
target_include_directories(test_visualize_raw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    #test_visualize_ae
add_executable(test_visualize_ae tests/test_visualize_ae.cpp)
target_link_libraries(test_visualize_ae PRIVATE event_lib)
target_include_directories(test_visualize_ae PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_test(NAME EventTests COMMAND test_event)
add_test(NAME BuildIncludeRunTests COMMAND test_build_include_run)
add_test(NAME EventPacketTests COMMAND test_event_packet)
add_test(NAME DatHTests COMMAND test_datH)
add_test(NAME RawTests COMMAND test_raw)
add_test(NAME AedatTests COMMAND test_aedat)
add_test(NAME VisualizingTestsDat COMMAND test_visualize)
add_test(NAME VisualizingTestsRaw COMMAND test_visualize_raw)
add_test(NAME VisualizingTestsAedat COMMAND test_visualize_ae)



# Optional: Add examples (if needed)
# add_subdirectory(examples)
