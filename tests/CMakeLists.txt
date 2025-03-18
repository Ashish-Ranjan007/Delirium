cmake_minimum_required(VERSION 3.16)
project(DeliriumTests LANGUAGES CXX)

enable_testing()

# Add GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
FetchContent_MakeAvailable(googletest)

set(TEST_SOURCES
    test_lexer.cpp
)

# Define test executable inside the test build directory
add_executable(delirium_tests ${TEST_SOURCES})

# Specify output directory for test binaries
set_target_properties(delirium_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Link with GoogleTest
target_link_libraries(delirium_tests PRIVATE gtest gtest_main)

# Register tests
add_test(NAME DeliriumTests COMMAND ${CMAKE_BINARY_DIR}/delirium_tests)
