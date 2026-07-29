# статическая
add_library(logger_static STATIC logger.cpp level.cpp)
target_include_directories(logger_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# динамическая
add_library(logger_shared SHARED logger.cpp level.cpp)
target_include_directories(logger_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
