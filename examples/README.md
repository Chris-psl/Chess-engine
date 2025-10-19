# Examples Directory

This directory contains example programs demonstrating how to use the chess engine.

## Structure

Each example demonstrates a specific feature or use case:

### basic_game.cpp
A simple example showing:
- Creating a chess board
- Making moves
- Displaying the board
- Basic game loop

### move_generator_demo.cpp
Demonstrates:
- Generating legal moves for a position
- Listing all possible moves
- Move validation

### position_evaluation.cpp
Shows:
- Evaluating chess positions
- Understanding evaluation scores
- Comparing different positions

### search_demo.cpp
Illustrates:
- Using the search algorithm
- Finding the best move
- Understanding search depth and time control

### uci_interface.cpp
Example of:
- Implementing UCI protocol communication
- Engine-GUI integration
- Command handling

## Building Examples

```bash
mkdir build && cd build
cmake -DBUILD_EXAMPLES=ON ..
make
```

## Running Examples

```bash
./basic_game
./move_generator_demo
./position_evaluation
```

## Adding New Examples

When adding new examples:
1. Create a new .cpp file in this directory
2. Update `examples/CMakeLists.txt`
3. Document the example's purpose in this README
