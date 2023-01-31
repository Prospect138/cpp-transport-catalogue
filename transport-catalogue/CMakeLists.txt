cmake_minimum_required(VERSION 3.13)  
project(transport_vs)           

set(SRC 
main.cpp
input_reader.cpp
stat_reader.cpp 
transport_catalogue.cpp
)

set(HDRы
geo.h
input_reader.h
stat_reader.h
transport_catalogue.h
)

add_executable(a ${SRC} ${HDR})
set_target_properties (a PROPERTIES
CXX_STANDARD 17
CXX_STANDARD_REQUIRED ON)