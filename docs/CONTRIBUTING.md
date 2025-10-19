# Contributing to Chess Engine

Thank you for your interest in contributing to this chess engine project!

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/your-username/Chess-engine.git
   cd Chess-engine
   ```
3. **Create a feature branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Setup

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or higher
- Git

### Building the Project

```bash
./tools/build.sh  # Unix/Linux/macOS
# or
tools\build.bat   # Windows
```

### Running Tests

```bash
cd build
ctest
```

## Code Style

Follow the project's coding conventions:

- Use 4 spaces for indentation (no tabs)
- Opening braces on the same line for functions and classes
- Use meaningful variable and function names
- Add comments for complex logic
- Follow the existing code structure

See `docs/CODING_STYLE.md` for detailed guidelines.

## Making Changes

### Adding New Features

1. Create appropriate header file in `include/`
2. Implement in corresponding `src/` directory
3. Add unit tests in `tests/unit/`
4. Update `CMakeLists.txt` if needed
5. Document your changes

### Bug Fixes

1. Write a test that reproduces the bug
2. Fix the bug
3. Verify the test passes
4. Document the fix in commit message

## Testing

- Write unit tests for all new functionality
- Ensure all existing tests pass
- Add integration tests for major features
- Test edge cases and boundary conditions

## Documentation

- Update relevant documentation in `docs/`
- Add docstrings to new classes and functions
- Update README.md if needed
- Include examples for new features

## Submitting Changes

1. **Commit your changes** with clear messages:
   ```bash
   git add .
   git commit -m "Add feature: brief description"
   ```

2. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```

3. **Create a Pull Request** on GitHub:
   - Provide a clear title and description
   - Reference any related issues
   - Describe what you changed and why

## Pull Request Guidelines

- Keep changes focused and atomic
- Ensure all tests pass
- Update documentation
- Follow the code style
- Be responsive to feedback

## Code Review Process

1. Maintainers will review your PR
2. Address any requested changes
3. Once approved, your PR will be merged

## Areas for Contribution

### High Priority
- Move generation implementation
- Basic evaluation function
- Search algorithm implementation
- UCI protocol implementation

### Medium Priority
- Opening book support
- Performance optimizations
- Additional evaluation features
- Enhanced UI

### Future Features
- Endgame tablebases
- Neural network evaluation
- Multi-threading support
- Advanced time management

## Questions?

Feel free to open an issue for:
- Bug reports
- Feature requests
- Questions about the code
- Discussion of implementation approaches

## Code of Conduct

- Be respectful and constructive
- Welcome newcomers
- Focus on the code, not the person
- Help others learn and grow

Thank you for contributing to make this chess engine better!
