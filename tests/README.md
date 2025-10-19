# Tests Directory

This directory contains all test files for the chess engine.

## Structure

### unit/
Unit tests for individual components:
- `board_test.cpp` - Board representation tests
- `move_generator_test.cpp` - Move generation tests
- `evaluation_test.cpp` - Evaluation function tests
- `search_test.cpp` - Search algorithm tests
- `uci_test.cpp` - UCI protocol tests

### integration/
Integration tests for complete workflows:
- `game_test.cpp` - Full game simulation tests
- `perft_test.cpp` - Performance tests (move generation verification)
- `tactical_test.cpp` - Tactical puzzle solving tests

## Testing Framework

The project uses Google Test (GTest) framework for unit and integration testing.

### Running Tests

```bash
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..
make
ctest
```

### Writing Tests

Follow the existing test structure and naming conventions:
- Test files should end with `_test.cpp`
- Test cases should use descriptive names
- Group related tests using TEST_F for fixtures
- Include edge cases and boundary conditions

## Test Coverage

Aim for high test coverage, especially for:
- Move generation correctness
- Evaluation function stability
- Search algorithm correctness
- UCI protocol compliance
