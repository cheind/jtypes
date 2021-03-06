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

set(LIB_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/jtypes/jtypes.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/jtypes/jtypes_io.hpp
)

set(LIB_INSTALL_FILES
    ${LIB_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/variant/variant.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nlohmann-json/src/json.hpp
)

set(LIB_LINK_TARGETS)

add_library(jtypes INTERFACE)
target_include_directories(jtypes INTERFACE ${LIB_INCLUDE_DIRS})
target_sources(jtypes INTERFACE ${LIB_HEADERS})

install(FILES ${LIB_INSTALL_FILES} DESTINATION inc/jtypes)


# Tests

set(TEST_SOURCES
    tests/catch.hpp
    tests/test_compile_units.cpp
    tests/test_jtypes.cpp
)

set(TEST_LINK_TARGETS
    jtypes
)

add_executable(jtypes-tests ${TEST_SOURCES})
target_link_libraries(jtypes-tests ${TEST_LINK_TARGETS})
