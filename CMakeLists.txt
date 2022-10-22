cmake_minimum_required(VERSION 3.16)
project(genicam_wrapper)

set(CMAKE_CXX_STANDARD 20)

find_package(genicam-mtca-ias 3.3.0 CONFIG REQUIRED)

add_subdirectory(src)
