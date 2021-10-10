cmake_minimum_required(VERSION 3.5)

project(scan)

set(SOURCES
    source/scan.cpp
    source/lexer.cpp
    source/token.cpp
)

add_executable(scan ${SOURCES})

target_include_directories(scan
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
set(CMAKE_CXXFLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")