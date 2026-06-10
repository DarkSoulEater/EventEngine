cmake_minimum_required(VERSION 3.16)
project(event-engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_SANDBOX "Build sandbox executable" ON)
option(BUILD_BENCHMARK "Build benchmark executable" ON)

include(FetchContent)

add_library(event STATIC)

#Fu2
FetchContent_Declare(
        function2
        GIT_REPOSITORY https://github.com/Naios/function2.git
        GIT_TAG 4.1.0
        GIT_SHALLOW TRUE
)
set(FUNCTION2_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(function2)
target_link_libraries(event PUBLIC
        function2::function2
)

# Google Benchmark
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable building tests" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable building gtest tests" FORCE)

FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.5
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(benchmark)

# ASIO
find_package(Threads REQUIRED)
target_link_libraries(event PRIVATE Threads::Threads)

target_compile_definitions(event PRIVATE ASIO_STANDALONE)

FetchContent_Declare(
        asio_standalone
        URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-30-2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(asio_standalone)
target_include_directories(event PUBLIC ${asio_standalone_SOURCE_DIR}/asio/include)

# SRC/HDR
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/core/src/*.cpp"
)

file(GLOB_RECURSE PUBLIC_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/core/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/include/*.hpp"
)

file(GLOB_RECURSE PRIVATE_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/core/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/src/*.hpp"
)

# Event Lib
target_sources(event
        PRIVATE
        ${SOURCES}
        ${PRIVATE_HEADERS}
        PUBLIC
        ${PUBLIC_HEADERS}
)

target_include_directories(event
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core/include>
        PRIVATE
        core/src
)

# Sandbox
if (BUILD_SANDBOX)
    file(GLOB SANDBOX_FILES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/sandbox/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/sandbox/*.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/sandbox/*.hpp"
    )

    add_executable(sandbox)

    target_sources(sandbox
            PRIVATE
            ${SANDBOX_FILES}
    )

    target_link_libraries(sandbox
            PRIVATE
            event
    )
endif ()

# Perf test
if (BUILD_BENCHMARK)
    file(GLOB BENCHMARK_FILES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/benchmark/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/benchmark/*.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/benchmark/*.hpp"
    )

    add_executable(event-benchmark)

    target_sources(event-benchmark
            PRIVATE
            ${BENCHMARK_FILES}
    )

    target_link_libraries(event-benchmark
            PRIVATE
            event
            benchmark::benchmark
    )
endif ()