#!/bin/bash
set -e  # Exit immediately if a command exits with a non-zero status

TEST_BUILD_DIR="build_tests"
TEST_BINARY="$TEST_BUILD_DIR/delirium_tests"

# Ensure the build directory exists
if [ ! -d "$TEST_BUILD_DIR" ]; then
    echo "Test build directory not found. Creating..."
    mkdir -p "$TEST_BUILD_DIR"
fi

# Build tests if the test binary does not exist
if [ ! -f "$TEST_BINARY" ]; then
    echo "Test executable not found. Building tests..."
    cmake -S tests -B "$TEST_BUILD_DIR"
    cmake --build "$TEST_BUILD_DIR"
fi

# Run tests
cd "$TEST_BUILD_DIR"
ctest --output-on-failure
