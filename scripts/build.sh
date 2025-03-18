#!/bin/bash
set -e  # Exit if any command fails

# Navigate to project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

echo "==> Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "==> Running CMake..."
cmake -DCMAKE_CXX_COMPILER=clang++ "$PROJECT_ROOT"  # Explicitly set the source directory

echo "==> Compiling..."
make -j$(nproc)

echo "==> Build complete!"
