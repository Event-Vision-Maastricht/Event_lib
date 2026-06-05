
```powershell
cmake -S experiments\scalability -B build-scalability -DEVENT_LIB_SOURCE_DIR=C:\Users\user\Desktop\okul\thesi\Event_lib
cmake --build build-scalability --config Release

.\build-scalability\Release\parser_benchmark.exe C:\path\to\file.raw 1000000 10000 parser_results.xml 5

.\build-scalability\Release\representation_benchmark.exe C:\path\to\file.dat 1000000 event_count 10000 10000 representation_results.xml 0 5
.\build-scalability\Release\representation_benchmark.exe C:\path\to\file.dat 1000000 time_window 16 10000 representation_results.xml 0 5
.\build-scalability\Release\representation_benchmark.exe C:\path\to\file.dat 1000000 time_surface 0 10000 representation_results.xml 0 5
```

```powershell
PARSER:
    100k evts 10k packet
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.dat 100000 10000 experiments\results\parser_100e_10p_dat.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.raw 100000 10000 experiments\results\parser_100e_10p_raw.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 100000 10000 experiments\results\parser_100e_10p_ae.xml 5
    1m evts 10k packet
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.dat 1000000 10000 experiments\results\parser_1me_10p_dat.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.raw 1000000 10000 experiments\results\parser_1me_10p_raw.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 10000 experiments\results\parser_1me_10p_ae.xml 5
    10m evts 10k packet
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.dat 10000000 10000 experiments\results\parser_10me_10p_dat.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.raw 10000000 10000 experiments\results\parser_1me_10p_raw.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 10000000 10000 experiments\results\parser_1me_10p_ae.xml 5

    1m evts 5k packet
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.dat 1000000 10000 experiments\results\parser_5p_dat.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.raw 1000000 10000 experiments\results\parser_5p_raw.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 10000 experiments\results\parser_5p_ae.xml 5
    1m evts 10k packet
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.dat 1000000 10000 experiments\results\parser_10p_dat.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.raw 1000000 10000 experiments\results\parser_10p_raw.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 10000 experiments\results\parser_10p_ae.xml 5
    1m evts 30k packet
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.dat 1000000 10000 experiments\results\parser_30p_dat.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\spinner.raw 1000000 10000 experiments\results\parser_30p_raw.xml 5
.\build-scalability\Release\parser_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 10000 experiments\results\parser_30p_ae.xml 5




FRAME GENERATION:(on aedat file)
evt-count
    100k evts, 10k read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 100000 event_count 10000 10000 experiments\results\frame_evt_100k_10p.xml 0 5
    1m events, 10k read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 event_count 10000 10000 experiments\results\frame_evt_1m_10p.xml 0 5
    10m evts, 10k read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 10000000 event_count 10000 10000 experiments\results\frame_evt_10m_10p.xml 0 5

PACKET AMOUNT CHANGE
-evt
    1m events, 10k read 5k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 event_count 10000 30000 experiments\results\frame_evt_10r_5p.xml 0 5
    1m events, 10k read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 event_count 10000 10000 experiments\results\frame_evt_10r_10p.xml 0 5
    1m events, 10k read 30k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 event_count 10000 30000 experiments\results\frame_evt_10r_30p.xml 0 5
-time
    time window 1m events, 16ms read 5k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 time_window 16000 10000 experiments\results\frame_tw_16ms_5p.xml 0 5
    time window 1m events, 16ms read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 time_window 16000 10000 experiments\results\frame_tw_16ms_10p.xml 0 5
    time window 1m events, 16ms read 30k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 time_window 16000 10000 experiments\results\frame_tw_16ms_30p.xml 0 5


time-window
    time window 100k events, 16ms read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 100000 time_window 16000 10000 experiments\results\frame_tw_100k_16ms10p.xml 0 5
    time window 1m events, 16ms read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 time_window 16000 10000 experiments\results\frame_tw_1m_16ms10p.xml 0 5
    time window 10m events, 16ms read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 10000000 time_window 16000 10000 experiments\results\frame_tw_10m_16ms10p.xml 0 5

    time window 1m events, 8ms read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 time_window 8000 10000 experiments\results\frame_tw_1m_8ms10p.xml 0 5
    time window 1m events, 33ms read 10k packet:
.\build-scalability\Release\representation_benchmark.exe C:\Users\user\Desktop\okul\thesi\data\a.aedat 1000000 time_window 33000 10000 experiments\results\frame_tw_1m_33ms10p.xml 0 5


```