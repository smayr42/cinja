cmake_minimum_required(VERSION 2.8.4)
project(example)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -O")

find_package(PkgConfig REQUIRED)
pkg_check_modules(PION REQUIRED pion)
include_directories(${PION_INCLUDE_DIRS})
link_directories(${PION_LIBRARY_DIRS})
add_definitions(${PION_CFLAGS_OTHER})

add_library(example SHARED handler.cpp)

set_target_properties(example PROPERTIES PREFIX "")
target_link_libraries(example ${PION_LIBRARIES})
