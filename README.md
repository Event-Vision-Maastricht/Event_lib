# Event_lib
Tools for simplifying your event-based life

Overleaf link: https://www.overleaf.com/read/hhtprzkmqqfg#3263d1

# Dependencies
- CMake 3.15 or newer
- C++17 compiler
  - Windows: MSVC / Visual Studio Build Tools
  - Linux: GCC or Clang
- OpenCV, only needed when `EVENT_LIB_WITH_OPENCV=ON`

## TODO:
- aedat file parser
- linux support
- camera parser/stream

#### supported data file formats:
- raw version EVT2.0
- dat CD event type
- aedat AEDAT2.0 DVS type


## CMake options

- `EVENT_LIB_WITH_OPENCV=ON`: builds OpenCV window visualization support. This is ON by default.
- `EVENT_LIB_BUILD_VISUAL_TESTS=ON`: builds the OpenCV window/manual visualization tests. This is OFF by default.
- `BUILD_TESTING=ON`: builds the normal test executables. This is usually ON by default when using CTest.


## Build on Windows

Run commands from the `Event_lib` folder.

### Build library and normal tests

If OpenCV is available through your environment:

```powershell
cmake -S . -B build -DEVENT_LIB_WITH_OPENCV=ON -DBUILD_TESTING=ON
cmake --build build --config Debug
```

If CMake cannot find OpenCV, pass the folder that contains `OpenCVConfig.cmake`:

```powershell
cmake -S . -B build -DEVENT_LIB_WITH_OPENCV=ON -DBUILD_TESTING=ON -DOpenCV_DIR="C:\path\to\opencv\build"
cmake --build build --config Debug
```

If you are using a conda/cppEnv OpenCV install, activate it before configuring:

```powershell
conda activate cppEnv
cmake -S . -B build -DEVENT_LIB_WITH_OPENCV=ON -DBUILD_TESTING=ON
cmake --build build --config Debug
```

### Build everything, including visual tests

```powershell
cmake -S . -B build -DEVENT_LIB_WITH_OPENCV=ON -DEVENT_LIB_BUILD_VISUAL_TESTS=ON -DBUILD_TESTING=ON
cmake --build build --config Debug
```

Run all registered tests:

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

Run a specific executable:

```powershell
cd build\Debug
.\test_event.exe
.\test_visualize.exe
.\test_visualize_raw.exe
.\test_visualize_ae.exe
```

Visual tests are manual/window-based and are only built when `EVENT_LIB_BUILD_VISUAL_TESTS=ON`.


## Build on Linux

Run commands from the `Event_lib` folder.

### Install common dependencies

Ubuntu/Debian example:

```bash
sudo apt update
sudo apt install build-essential cmake
```

For OpenCV visualization support:

```bash
sudo apt install libopencv-dev
```

### Headless/parser-only build

Use this on servers, CI, or systems without OpenCV/window support:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DEVENT_LIB_WITH_OPENCV=OFF -DBUILD_TESTING=ON
cmake --build build -j
cd build
ctest --output-on-failure
cd ..
```

### Build with OpenCV support

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DEVENT_LIB_WITH_OPENCV=ON -DBUILD_TESTING=ON
cmake --build build -j
cd build
ctest --output-on-failure
cd ..
```

### Build visual tests on Linux

Visual tests require OpenCV HighGUI and a display server. Make sure `DISPLAY` or `WAYLAND_DISPLAY` is set.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DEVENT_LIB_WITH_OPENCV=ON -DEVENT_LIB_BUILD_VISUAL_TESTS=ON -DBUILD_TESTING=ON
cmake --build build -j
```

Run a visual test:

```bash
./build/test_visualize
./build/test_visualize_raw
./build/test_visualize_ae
```

When run directly, visual tests keep the window open after playback. Press `q` or `Esc` to close it.
CTest disables this hold automatically so automated test runs can finish.


## Install and consume with CMake

Install locally:

```bash
cmake --install build --prefix install
```

Use from another CMake project:

```cmake
find_package(event_lib CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE event_lib::event_lib)
```



---- future notes for me
#### compiler
--> gcc linux msvc windows, dont forget to arrange it in Cmake

    -> possible mac, clang default
producer-consumer pipeline.
