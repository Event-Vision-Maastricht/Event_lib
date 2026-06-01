# Event_lib
Tools for simplifying your event-based life

Overleaf link: https://www.overleaf.com/read/hhtprzkmqqfg#3263d1

# Dependencies
- Opencv (cppEnv)

## TODO:
- aedat file parser
- linux support
- camera parser/stream




#### supported data file formats:
- raw version EVT2.0
- dat CD event type
- aedat AEDAT2.0 DVS type


#### build and testing for windows
To build (assuming you are in Event_lib folder):
```
cmake --build build --config Debug
```

running tests (assuming you are in Event_lib folder):
```
cd build 
ctest verbose -C debug
```

running a specific test:
```
cd build/debug
.\<test name>.exe
```



---- future notes for me
#### compiler
--> gcc linux msvc windows, dont forget to arrange it in Cmake

    -> possible mac, clang default
producer-consumer pipeline.
