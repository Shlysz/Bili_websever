cmake_minimum_required(VERSION 3.23)
project(Bili_websever)

set(CMAKE_CXX_STANDARD 14)

link_libraries(ws2_32)
add_executable(Bili_websever http.cpp)
