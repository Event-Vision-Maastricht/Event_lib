# event_lib

A small C++17 library for working with event-camera data.

`event_lib` provides simple building blocks for reading event streams, grouping
events into packets, accessing sensor metadata, and optionally visualizing event
frames with OpenCV.

## Features

- Core `Event` and `EventPacket` types
- Parser interface for event-camera files
- Dataset stream API for sequential event reading
- Shared `SensorMetadata` structure
- Optional OpenCV frame/window visualization
- CMake install support with `find_package(event_lib CONFIG REQUIRED)`

## Supported Formats

| Format    |         Notes           |
|   ---     |         ---             |
| `.raw`    | EVT2.0 raw event files  |
| `.dat`    | CD event type DAT files |
| `.aedat`  | AEDAT 2.0 DVS files     |

## Requirements

- CMake 3.15 or newer
- A C++17 compiler
  - Windows: MSVC / Visual Studio Build Tools
  - Linux: GCC or Clang
- OpenCV, only when `EVENT_LIB_WITH_OPENCV=ON`

## Quick Start

Configure and build:

```bash
cmake -S . -B build -DEVENT_LIB_WITH_OPENCV=OFF -DBUILD_TESTING=ON
cmake --build build
```

Run the test suite:

```bash
cd build
ctest --output-on-failure
```

On Windows with a multi-config generator, include the build configuration:

```powershell
cmake -S . -B build -DEVENT_LIB_WITH_OPENCV=OFF -DBUILD_TESTING=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

## Basic Usage

```cpp
#include <iostream>
#include "event_lib/io/stream/DatasetEventStream.hpp"

int main() {
    event_lib::DatasetEventStream stream("events.dat");

    const auto& metadata = stream.metadata();
    std::cout << "Sensor: " << metadata.width << "x" << metadata.height << '\n';

    while (stream.has_next()) {
        event_lib::EventPacket packet = stream.next_packet(10000);

        for (const auto& event : packet.get_events()) {
            // event.timestamp, event.x, event.y, event.polarity
        }
    }

    stream.close();
    return 0;
}
```

## CMake Options

| Option | Default | Description |
| --- | --- | --- |
| `EVENT_LIB_WITH_OPENCV` | `ON` | Build OpenCV-based visualization support. |
| `EVENT_LIB_BUILD_VISUAL_TESTS` | `OFF` | Build manual/window-based visualization tests. Requires OpenCV. |
| `BUILD_TESTING` | CTest default | Build the normal test executables. |

`EVENT_LIB_VISUAL_HOLD=0` can be set in the environment to make visual tests
exit automatically. When it is unset, directly run visual tests keep their
window open until `q` or `Esc` is pressed.

## Building With OpenCV

If OpenCV is installed and discoverable by CMake:

```bash
cmake -S . -B build -DEVENT_LIB_WITH_OPENCV=ON -DBUILD_TESTING=ON
cmake --build build
```

If CMake cannot find OpenCV, pass the directory containing `OpenCVConfig.cmake`:

```powershell
cmake -S . -B build -DEVENT_LIB_WITH_OPENCV=ON -DBUILD_TESTING=ON -DOpenCV_DIR="C:\path\to\opencv\build"
cmake --build build --config Debug
```

## Visual Tests

Visual tests are only built when both OpenCV and
`EVENT_LIB_BUILD_VISUAL_TESTS=ON` are enabled:

```bash
cmake -S . -B build -DEVENT_LIB_WITH_OPENCV=ON -DEVENT_LIB_BUILD_VISUAL_TESTS=ON -DBUILD_TESTING=ON
cmake --build build
```

Run them directly from the build directory:

```bash
./test_visualize
./test_visualize_raw
./test_visualize_ae
```

On Windows, the executables are usually under the configuration directory:

```powershell
.\build\Debug\test_visualize.exe
.\build\Debug\test_visualize_raw.exe
.\build\Debug\test_visualize_ae.exe
```

## Install and Use From Another Project

Install the library:

```bash
cmake --install build --prefix install
```

Use it from another CMake project:

```cmake
find_package(event_lib CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE event_lib::event_lib)
```

If you installed to a custom prefix, point CMake at it when configuring your
application:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/event_lib/install
```

## Project Layout

```text
include/event_lib/      Public headers
src/                    Library implementation
tests/                  Unit and visual tests
cmake/                  Package configuration template
```

## Notes

- Parser selection is based on the input file extension.
- `.dat`, `.raw`, and `.aedat` parser tests may skip automatically when sample
  data files are not available.
- OpenCV is optional; turn it off for parser-only builds, servers, or CI jobs
  without display support.
