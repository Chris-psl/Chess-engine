#!/bin/bash
# Quick build script for Unix systems

set -e  # Exit on error

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building Chess Engine...${NC}"

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
echo -e "${GREEN}Configuring with CMake...${NC}"
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON ..

# Build
echo -e "${GREEN}Compiling...${NC}"
make -j$(nproc)

echo -e "${GREEN}Build completed successfully!${NC}"
