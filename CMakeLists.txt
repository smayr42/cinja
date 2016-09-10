cmake_minimum_required(VERSION 2.8.4)
project(cinja)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -O")

set(SOURCE_FILES
    src/main.cpp
    src/parser.cpp
    src/token.cpp
    src/lexer.cpp
    src/validate.cpp
    src/tokens.cpp
    src/gen.cpp)

add_executable(cinja ${SOURCE_FILES})

