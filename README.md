# Event_lib
Tools for simplifying your event-based life

Overleaf link: https://www.overleaf.com/read/hhtprzkmqqfg#3263d1


---- future notes for me
#### compiler
--> gcc linux msvc windows, dont forget to arrange it in Cmake
    -> possible mac, clang default


#### build and testing for windows
To build (assuming you are in Event_lib folder):
'''
cmake --build build --config Debug
'''

running tests (assuming you are in Event_lib folder):
'''
cd build 
ctest verbose -C debug
'''
