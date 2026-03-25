cmake_minimum_required(VERSION 3.16)
project(event-engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_SANDBOX "Build sandbox executable" ON)

add_library(event STATIC)

file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/core/src/event/*.cpp"
)

file(GLOB PUBLIC_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/core/include/event/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/include/event/*.hpp"
)

file(GLOB PRIVATE_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/core/src/event/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/src/event/*.hpp"
)

target_sources(event
        PRIVATE
        ${SOURCES}
        ${PRIVATE_HEADERS}
        PUBLIC
        ${PUBLIC_HEADERS}
)

target_include_directories(event
        PUBLIC
        core/include
        PRIVATE
        core/src
)

if (BUILD_SANDBOX)
    file(GLOB SANDBOX_FILES
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