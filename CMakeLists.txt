cmake_minimum_required(VERSION 3.16)
project(LoggerProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build tests" ON)

add_subdirectory(lib)
add_subdirectory(app)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)  # тесты — последние
endif()