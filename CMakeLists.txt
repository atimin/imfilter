cmake_minimum_required(VERSION 3.15)
project(image_process)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(blaze)

FIND_PACKAGE(Threads REQUIRED)

include_directories(blaze)
add_executable(test test.cpp)
add_executable(test_image image_test.cpp CImg/CImg.h)
target_compile_definitions(test_image PRIVATE cimg_display=0)
target_link_libraries(test_image Threads::Threads)
