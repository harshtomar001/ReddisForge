cmake_minimum_required(VERSION 3.20)

project(RedisForge
VERSION 0.1.0
LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Main RedisForge application
add_executable(redisforge
        src/main.cpp
        src/core/storage_engine.cpp
        src/core/storage_engine.h
        src/server/command_executor.cpp
        src/server/command_executor.h
)

target_compile_options(redisforge PRIVATE
-Wall
-Wextra
-Wpedantic
)

# Testing infrastructure
include(CTest)

if(BUILD_TESTING)
find_package(GTest CONFIG REQUIRED)

# Separate executable for unit tests
add_executable(redisforge_tests
        tests/storage_engine/storage_engine_test.cpp
        src/core/storage_engine.cpp
        src/server/command_executor.cpp
        tests/server/command_executor_test.cpp
)

target_link_libraries(redisforge_tests
PRIVATE
GTest::gtest_main
)

target_compile_options(redisforge_tests PRIVATE
-Wall
-Wextra
-Wpedantic
)

include(GoogleTest)
gtest_discover_tests(redisforge_tests)
endif()
