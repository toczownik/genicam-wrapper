cmake_minimum_required(VERSION 3.16)
project(genicam_wrapper)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_finds")

find_package(GenICam REQUIRED)
include_directories(${GENICAM_LIBRARY_DIR})
include_directories(${GENICAM_INCLUDE_DIR})

add_subdirectory(src)
