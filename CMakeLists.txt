cmake_minimum_required(VERSION 3.0)

project(javascript-types)

# Compiler settings
set(CMAKE_CXX_STANDARD 11)
if (WIN32)
	add_definitions("-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS")
endif()

# Options

# Library

set(LIB_INCLUDE_DIRS
	inc
	vendor/variant
    vendor/nlohmann-json/src
)

set(LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/jtypes/jtypes.h
)

set(LIB_LINK_TARGETS)

add_library(jtypes INTERFACE)
target_include_directories(jtypes INTERFACE ${LIB_INCLUDE_DIRS})
target_sources(jtypes INTERFACE ${LIB_SOURCES})

# Tests

set(TEST_SOURCES
    tests/catch.hpp
    tests/test_jtypes.cpp
)

set(TEST_LINK_TARGETS
    jtypes
)

add_executable(jtypes-tests ${TEST_SOURCES})
target_link_libraries(jtypes-tests ${TEST_LINK_TARGETS})
