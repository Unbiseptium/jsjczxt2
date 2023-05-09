cmake_minimum_required(VERSION 3.25)
project(jsjczxt2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static")
add_executable(jsjczxt2 main.cpp basic.h)
