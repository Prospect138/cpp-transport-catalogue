cmake_minimum_required(VERSION 3.13)  
project(transport_vs)           

set(SRC 
main.cpp
domain.cpp
transport_catalogue.cpp
json.cpp
json_reader.cpp
map_renderer.cpp
svg.cpp
)

set(HDR
geo.h
domain.h
transport_catalogue.h
json.h
json_reader.h
map_renderer.h
svg.h
)

add_executable(a ${SRC} ${HDR})
set_target_properties (a PROPERTIES
CXX_STANDARD 17
CXX_STANDARD_REQUIRED ON)