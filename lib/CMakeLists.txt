cmake_minimum_required(VERSION 3.16)
project(lib VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(
    lib_files
    Logger/BlockingQueue.cpp
    Logger/LogLevel.cpp
    Logger/Logger.cpp
)

add_library(lib_static STATIC ${lib_files})
add_library(lib_shared SHARED ${lib_files})

target_include_directories(lib_static PUBLIC Logger)
target_include_directories(lib_shared PUBLIC Logger)
