# The experiment build for the library

## BUILD
```powershell
cmake -S experiments\scalability -B build-scalability -DEVENT_LIB_SOURCE_DIR=C:\Users\user\Desktop\okul\thesi\Event_lib
cmake --build build-scalability --config Release
```

## TESTS
```powershell
PARSER:
    100k evts 10k packet
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.dat 100000 10000 experiments\results\parser.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.raw 100000 10000 experiments\results\parser.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 100000 10000 experiments\results\parser.xml 5
    1m evts 10k packet
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.dat 1000000 10000 experiments\results\parser.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.raw 1000000 10000 experiments\results\parser.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 10000 experiments\results\parser.xml 5
    10m evts 10k packet
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.dat 10000000 10000 experiments\results\parser.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.raw 10000000 10000 experiments\results\parser.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 10000000 10000 experiments\results\parser.xml 5

    1m evts 5k packet
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.dat 1000000 10000 experiments\results\parser.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.raw 1000000 10000 experiments\results\parser.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 10000 experiments\results\parser.xml 5
    1m evts 30k packet
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.dat 1000000 10000 experiments\results\parser.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.raw 1000000 10000 experiments\results\parser.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 10000 experiments\results\parser.xml 5




FRAME GENERATION:(on aedat file)
evt-count
    100k evts, 10k read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 100000 event_count 10000 10000 experiments\results\frame_evt.xml 0 5
    1m events, 10k read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 event_count 10000 10000 experiments\results\frame_evt.xml 0 5
    10m evts, 10k read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 10000000 event_count 10000 10000 experiments\results\frame_evt.xml 0 5
    1m events, 10k read 5k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 event_count 10000 30000 experiments\results\frame_evt.xml 0 5
    1m events, 5k read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 event_count 5000 10000 experiments\results\frame_evt.xml 0 5
    1m events, 10k read 30k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 event_count 10000 30000 experiments\results\frame_evt.xml 0 5
    1m events, 30k read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 event_count 30000 10000 experiments\results\frame_evt.xml 0 5


time-window
    time window 1m events, 16ms read 5k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 time_window 16000 10000 experiments\results\frame_tw.xml 0 5
    time window 1m events, 16ms read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 time_window 16000 10000 experiments\results\frame_tw.xml 0 5
    time window 1m events, 16ms read 30k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 time_window 16000 10000 experiments\results\frame_tw.xml 0 5
    time window 100k events, 16ms read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 100000 time_window 16000 10000 experiments\results\frame_tw.xml 0 5
    time window 10m events, 16ms read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 10000000 time_window 16000 10000 experiments\results\frame_tw.xml 0 5
    time window 1m events, 8ms read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 time_window 8000 10000 experiments\results\frame_tw.xml 0 5
    time window 1m events, 33ms read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 time_window 33000 10000 experiments\results\frame_tw.xml 0 5


```