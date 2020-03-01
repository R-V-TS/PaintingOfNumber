cmake_minimum_required(VERSION 3.15)
project(ImageDraw)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)

add_executable(ImageDraw main.cpp)

target_link_libraries(ImageDraw ${OpenCV_LIBS})