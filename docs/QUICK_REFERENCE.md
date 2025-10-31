# Quick Reference Guide

Quick reference for common tasks and commands in the Chess Engine project.

## Building

```bash
# Quick build (Unix/Linux/macOS)
./tools/build.sh

# Clean build
./tools/clean.sh
./tools/build.sh

# Manual build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## Testing

```bash
# Run all tests
cd build
ctest

# Run with verbose output
ctest --verbose

# Run specific test
ctest -R unit_tests

# Run tests with output
ctest --output-on-failure
```

## Documentation

```bash
# Generate documentation
cd docs
doxygen Doxyfile

# View documentation
open html/index.html  # macOS
xdg-open html/index.html  # Linux
```

## Code Formatting

```bash
# Format all code
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Check formatting (dry run)
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run -Werror
```

## Git Workflow

```bash
# Create feature branch
git checkout -b feature/my-feature

# Add and commit changes
git add .
git commit -m "Description of changes"

# Push to fork
git push origin feature/my-feature

# Update from main
git fetch upstream
git merge upstream/main
```

## Useful Commands

### CMake Options

```bash
# Build with tests
cmake -DBUILD_TESTS=ON ..

# Build with examples
cmake -DBUILD_EXAMPLES=ON ..

# Build everything
cmake -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON ..

# Set compiler
cmake -DCMAKE_CXX_COMPILER=clang++ ..
```

### Finding Files

```bash
# Find all source files
find src -name "*.cpp"

# Find all header files
find include -name "*.h"

# Find TODO comments
grep -r "TODO" src/ include/
```

### Code Analysis

```bash
# Static analysis with clang-tidy
clang-tidy src/**/*.cpp -- -I include/

# Check for memory leaks with valgrind
valgrind --leak-check=full ./chess_engine

# Profile with gprof
gprof ./chess_engine gmon.out > analysis.txt

# Profile with perf (Linux)
perf record ./chess_engine
perf report
```

## Directory Quick Navigation

```bash
# Source files
cd src/board        # Board representation
cd src/moves        # Move generation
cd src/evaluation   # Evaluation
cd src/search       # Search algorithms
cd src/engine       # Engine core
cd src/ui           # User interface

# Header files
cd include/board
cd include/moves
# ... (mirrors src structure)

# Tests
cd tests/unit
cd tests/integration

# Documentation
cd docs
```

## Common File Locations

| File Type | Location | Purpose |
|-----------|----------|---------|
| Header files | `include/*/` | Class declarations |
| Implementation | `src/*/` | Class implementations |
| Unit tests | `tests/unit/` | Component tests |
| Integration tests | `tests/integration/` | Full system tests |
| Examples | `examples/` | Usage examples |
| Documentation | `docs/` | Project documentation |
| Scripts | `tools/` | Build and utility scripts |
| Build files | `build/` | Generated build artifacts |

## CMake Targets

```bash
# When targets are defined:
make chess_engine        # Main executable
make chess_engine_lib    # Static library
make unit_tests          # Unit test suite
make integration_tests   # Integration test suite
make basic_game          # Example: basic game
```

## Include Paths

In code, include headers with:
```cpp
#include <board/board.h>
#include <moves/move_generator.h>
#include <evaluation/evaluator.h>
#include <search/search.h>
#include <engine/engine.h>
```

## Documentation Comments

### File header
```cpp
/**
 * @file filename.h
 * @brief Brief description
 */
```

### Class documentation
```cpp
/**
 * @class ClassName
 * @brief Brief description
 * 
 * Detailed description
 */
```

### Function documentation
```cpp
/**
 * @brief Brief description
 * @param param1 Description of param1
 * @param param2 Description of param2
 * @return Description of return value
 */
```

## Environment Variables

```bash
# Number of parallel jobs for make
export MAKEFLAGS="-j$(nproc)"

# Compiler selection
export CXX=clang++
export CC=clang
```

## Troubleshooting

### CMake cache issues
```bash
rm -rf build/CMakeCache.txt
rm -rf build/CMakeFiles
```

### Compiler issues
```bash
# Specify compiler explicitly
export CXX=g++-11
cmake ..
```

### Permission issues
```bash
chmod +x tools/*.sh
```

### IDE Integration

#### VS Code
- Install C/C++ extension
- Open folder
- CMake Tools will detect project

#### CLion
- Open CMakeLists.txt
- CLion auto-configures

#### vim/neovim
- Use compile_commands.json for completion
- Install ccls or clangd

## Performance Tips

- Use Release build for performance testing
- Enable compiler optimizations: `-O3`
- Profile before optimizing
- Use appropriate data structures
- Consider cache-friendly code

## Resources

- [Project README](../README.md)
- [Architecture](ARCHITECTURE.md)
- [Build Instructions](BUILD.md)
- [Contributing Guidelines](CONTRIBUTING.md)
- [Coding Style](CODING_STYLE.md)
- [Roadmap](ROADMAP.md)
