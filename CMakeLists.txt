cmake_minimum_required(VERSION 3.10)
project(LibStreamX C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(STREAMX_ASAN "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

if(STREAMX_ASAN AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g -O1)
    add_link_options(-fsanitize=address,undefined)
endif()

include_directories(include)

set(LIB_SOURCES
    src/ringbuf.c
    src/packet.c
    src/parser.c
    src/logger.c
    src/arena.c
    src/config.c
    src/cli.c
)

add_library(streamx STATIC ${LIB_SOURCES})

# Per-module test sources: every tests/test_*.c is compiled into the
# single test_runner executable. test_main.c is the aggregator entry point.
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "tests/test_*.c")
add_executable(test_runner ${TEST_SOURCES})
target_include_directories(test_runner PRIVATE tests)
target_link_libraries(test_runner streamx)

add_executable(streamx-cli src/main.c)
target_link_libraries(streamx-cli streamx)

enable_testing()
add_test(NAME LibStreamXTests COMMAND test_runner)
