cmake_minimum_required(VERSION 3.15)
project(GausFilter)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(blaze)

include_directories(blaze)
add_executable(GausFilter main.cpp)