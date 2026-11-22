project(app VERSION 1.0 LANGUAGES CXX)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE lib_static)