# Tools and Scripts Directory

This directory contains utility scripts and tools for development.

## Contents

### Build Scripts
- `build.sh` - Quick build script for Unix systems
- `build.bat` - Quick build script for Windows
- `clean.sh` - Clean build artifacts

### Testing Scripts
- `run_tests.sh` - Run all tests with coverage
- `perft.sh` - Run perft (performance test) suite

### Development Tools
- `format_code.sh` - Format code using clang-format
- `lint.sh` - Run static analysis tools
- `benchmark.sh` - Run performance benchmarks

### CI/CD
- `ci_build.sh` - Continuous integration build script
- `ci_test.sh` - Continuous integration test script

## Usage

Make scripts executable:
```bash
chmod +x tools/*.sh
```

Run scripts from project root:
```bash
./tools/build.sh
./tools/run_tests.sh
```

## Adding New Tools

When adding new tools or scripts:
1. Document the tool's purpose and usage in this README
2. Make scripts executable
3. Use consistent naming conventions
4. Add error handling and helpful output messages
