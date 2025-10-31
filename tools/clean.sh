#!/bin/bash
# Clean build artifacts

set -e  # Exit on error

echo "Cleaning build artifacts..."

# Remove build directory
if [ -d "build" ]; then
    rm -rf build
    echo "Removed build/ directory"
fi

# Remove any other build artifacts
find . -name "*.o" -delete
find . -name "*.a" -delete
find . -name "*.so" -delete
find . -name "CMakeCache.txt" -delete
find . -name "cmake_install.cmake" -delete
find . -type d -name "CMakeFiles" -exec rm -rf {} + 2>/dev/null || true

echo "Clean completed!"
