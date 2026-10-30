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
        src/server/resp2_encoder.cpp
        src/server/resp2_encoder.h
        src/server/resp2_parser.cpp
        src/server/resp2_parser.h
        src/server/resp2_value.h
        src/server/command_processor.cpp
        src/server/command_processor.h
        src/server/tcp_server.cpp
        src/server/tcp_server.h
        src/server/resp2_framer.cpp
        src/server/resp2_framer.h
)

target_compile_definitions(redisforge PRIVATE _WIN32_WINNT=0x0A00)
target_link_libraries(redisforge PRIVATE ws2_32 mswsock)
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
        src/server/resp2_parser.cpp
        src/server/resp2_encoder.cpp
        src/server/command_processor.cpp
        src/server/resp2_framer.cpp

        tests/server/command_executor_test.cpp
        tests/server/resp2_parser_test.cpp
        tests/server/resp2_encoder_test.cpp
        tests/server/command_processor_test.cpp
        tests/server/resp2_framer_test.cpp
)

target_link_libraries(redisforge_tests
PRIVATE
GTest::gtest_main
)

target_compile_definitions(redisforge_tests PRIVATE _WIN32_WINNT=0x0A00)
target_link_libraries(redisforge_tests PRIVATE ws2_32)

target_compile_options(redisforge_tests PRIVATE
-Wall
-Wextra
-Wpedantic

)

include(GoogleTest)
gtest_discover_tests(redisforge_tests)
endif()
