# Build Instructions

This document provides detailed instructions for building the Chess Engine project.

## Prerequisites

### Required

- **C++ Compiler**: GCC 7+, Clang 5+, or MSVC 2017+
- **CMake**: Version 3.14 or higher
- **Git**: For version control

### Optional

- **Google Test**: For running tests (can be auto-downloaded by CMake)
- **Doxygen**: For generating documentation
- **clang-format**: For code formatting

## Platform-Specific Setup

### Linux (Ubuntu/Debian)

```bash
# Install build tools
sudo apt-get update
sudo apt-get install build-essential cmake git

# Optional: Install testing framework
sudo apt-get install libgtest-dev

# Optional: Install documentation tools
sudo apt-get install doxygen graphviz
```

### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake (using Homebrew)
brew install cmake

# Optional: Install testing framework
brew install googletest

# Optional: Install documentation tools
brew install doxygen graphviz
```

### Windows

1. Install Visual Studio 2017 or later with C++ support
2. Install CMake from https://cmake.org/download/
3. Install Git from https://git-scm.com/download/win

## Building the Project

### Quick Build (Unix/Linux/macOS)

```bash
# Clone the repository
git clone https://github.com/Chris-psl/Chess-engine.git
cd Chess-engine

# Build using the provided script
./tools/build.sh
```

### Manual Build

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build .

# Or use make directly
make -j$(nproc)  # Linux/macOS
```

### Build with Options

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..

# With tests
cmake -DBUILD_TESTS=ON ..

# With examples
cmake -DBUILD_EXAMPLES=ON ..

# Custom install prefix
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
```

### Windows (Visual Studio)

```bash
# Open Visual Studio Command Prompt
mkdir build
cd build

# Configure for Visual Studio
cmake -G "Visual Studio 16 2019" ..

# Build
cmake --build . --config Release
```

## Build Targets

### Main Executable

```bash
# Build the chess engine executable
make chess_engine
```

### Library

```bash
# Build as a static library
make chess_engine_lib
```

### Tests

```bash
# Build and run tests
make test
# or
ctest

# Run with verbose output
ctest --verbose

# Run specific test
ctest -R unit_tests
```

### Examples

```bash
# Build all examples
make examples

# Build specific example
make basic_game
```

### Documentation

```bash
# Generate Doxygen documentation
cd docs
doxygen Doxyfile

# View documentation
open html/index.html  # macOS
xdg-open html/index.html  # Linux
```

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTS` | ON | Build test suite |
| `BUILD_EXAMPLES` | ON | Build example programs |
| `CMAKE_BUILD_TYPE` | - | Debug, Release, RelWithDebInfo, MinSizeRel |
| `CMAKE_INSTALL_PREFIX` | /usr/local | Installation directory |

## Troubleshooting

### CMake can't find compiler

```bash
# Specify compiler explicitly
cmake -DCMAKE_CXX_COMPILER=g++ ..
# or
cmake -DCMAKE_CXX_COMPILER=clang++ ..
```

### Missing dependencies

```bash
# Clean build directory and try again
rm -rf build
mkdir build
cd build
cmake ..
```

### Tests fail to build

```bash
# Build without tests
cmake -DBUILD_TESTS=OFF ..
```

### Permission denied on scripts

```bash
# Make scripts executable
chmod +x tools/*.sh
```

## Clean Build

```bash
# Remove build artifacts
./tools/clean.sh

# Or manually
rm -rf build
```

## Installation

```bash
# Install to system (may require sudo)
cd build
sudo make install

# Install to custom location
cmake -DCMAKE_INSTALL_PREFIX=$HOME/chess-engine ..
make install
```

## Development Build

For active development, use:

```bash
# Debug build with all features
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_TESTS=ON \
      -DBUILD_EXAMPLES=ON \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      ..
```

This creates `compile_commands.json` for IDE integration.

## Continuous Integration

The project includes CI scripts in `tools/`:

```bash
# Run CI build locally
./tools/ci_build.sh

# Run CI tests locally
./tools/ci_test.sh
```

## Performance Profiling

```bash
# Build with profiling enabled
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make

# Profile with gprof (Linux)
gprof ./chess_engine gmon.out > analysis.txt
```

## Further Help

- Check `docs/CONTRIBUTING.md` for contribution guidelines
- Open an issue on GitHub for build problems
- See project README for general information
