include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# здесь создаются таргеты GTest::gtest и GTest::gtest_main
FetchContent_MakeAvailable(googletest)

add_executable(logger_tests
    test_log_level.cpp
    test_blocking_queue.cpp
    test_logger.cpp
)

target_link_libraries(logger_tests PRIVATE lib_static GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(logger_tests)