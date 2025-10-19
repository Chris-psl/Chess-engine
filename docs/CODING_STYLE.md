# Coding Style Guide

This document outlines the C++ coding standards for the Chess Engine project.

## General Principles

1. **Readability**: Code should be easy to read and understand
2. **Consistency**: Follow existing patterns in the codebase
3. **Simplicity**: Keep code simple and straightforward
4. **Documentation**: Comment complex logic, not obvious code

## Naming Conventions

### Classes and Structs

Use PascalCase for class and struct names:

```cpp
class ChessBoard { };
class MoveGenerator { };
struct Position { };
```

### Functions and Methods

Use camelCase for function and method names:

```cpp
void makeMove(const Move& move);
bool isLegalMove(const Move& move);
int evaluatePosition();
```

### Variables

Use camelCase for local variables and parameters:

```cpp
int pieceCount = 0;
Move bestMove;
Square targetSquare;
```

### Member Variables

Prefix private member variables with `m_`:

```cpp
class Board {
private:
    Piece m_pieces[64];
    int m_moveCount;
    bool m_whiteToMove;
};
```

### Constants

Use UPPER_CASE with underscores:

```cpp
const int MAX_DEPTH = 20;
const int INFINITY_SCORE = 30000;
const char STARTING_FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
```

### Enums

Use PascalCase for enum names, UPPER_CASE for values:

```cpp
enum class PieceType {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NONE
};

enum Color {
    WHITE,
    BLACK
};
```

## Formatting

### Indentation

- Use 4 spaces (no tabs)
- Indent code blocks consistently

```cpp
if (condition) {
    doSomething();
    doAnotherThing();
}
```

### Braces

Opening braces on the same line for functions, classes, and control structures:

```cpp
class MyClass {
    void myFunction() {
        if (condition) {
            // code
        } else {
            // code
        }
    }
};
```

### Line Length

- Aim for 80-100 characters per line
- Break long lines at logical points

```cpp
// Good
bool result = isValidMove(move) && 
              !isInCheck(color) && 
              hasLegalMoves(position);

// Avoid
bool result = isValidMove(move) && !isInCheck(color) && hasLegalMoves(position) && someOtherCondition();
```

### Spacing

- One space after keywords: `if (`, `for (`, `while (`
- No space between function name and parentheses: `makeMove(`
- Space around operators: `a + b`, `x = y`
- No space before semicolons

```cpp
// Good
if (x > 0) {
    y = x + 5;
    makeMove(move);
}

// Avoid
if(x>0){
    y=x+5;
    makeMove (move);
}
```

## Comments

### Header Comments

Include a brief description at the top of each file:

```cpp
/**
 * @file board.h
 * @brief Chess board representation and state management
 * @author Chess Engine Team
 */
```

### Class Comments

Document classes with their purpose:

```cpp
/**
 * @class Board
 * @brief Represents a chess board and its current state
 * 
 * Manages piece positions, game state, and provides methods
 * for making and unmaking moves.
 */
class Board {
    // ...
};
```

### Function Comments

Document public functions:

```cpp
/**
 * @brief Evaluates the current board position
 * @param board The board to evaluate
 * @return Evaluation score (positive favors white)
 */
int evaluatePosition(const Board& board);
```

### Inline Comments

Use sparingly for complex logic:

```cpp
// Calculate mobility bonus for knights
// Knights are worth more when they have more available moves
int mobilityBonus = legalMoves.size() * 10;
```

## Header Files

### Include Guards

Use `#pragma once`:

```cpp
#pragma once

// or traditional guards:
#ifndef BOARD_H
#define BOARD_H

// declarations

#endif // BOARD_H
```

### Include Order

1. Related header (for .cpp files)
2. C system headers
3. C++ standard library headers
4. Project headers

```cpp
#include "board/board.h"  // Related header

#include <cstdint>        // C system
#include <cstring>

#include <vector>         // C++ standard library
#include <string>

#include <moves/move.h>   // Project headers
#include <evaluation/evaluator.h>
```

## Best Practices

### Use of auto

Use `auto` when the type is obvious or verbose:

```cpp
// Good
auto it = container.begin();
auto piece = board.getPieceAt(square);

// Avoid when type is not obvious
auto x = getValue();  // What type is x?
```

### Const Correctness

Use `const` wherever possible:

```cpp
// Member functions that don't modify state
int getScore() const;

// Parameters that shouldn't be modified
void makeMove(const Move& move);

// Variables that shouldn't change
const int maxDepth = 20;
```

### nullptr vs NULL

Use `nullptr` instead of `NULL`:

```cpp
Piece* piece = nullptr;  // Good
Piece* piece = NULL;     // Avoid
```

### Range-based for loops

Prefer range-based loops when possible:

```cpp
// Good
for (const auto& move : moves) {
    evaluateMove(move);
}

// Instead of
for (size_t i = 0; i < moves.size(); ++i) {
    evaluateMove(moves[i]);
}
```

### Error Handling

Use exceptions for exceptional cases:

```cpp
if (!isValidFEN(fen)) {
    throw std::invalid_argument("Invalid FEN string");
}
```

### Memory Management

Prefer smart pointers over raw pointers:

```cpp
std::unique_ptr<Board> board = std::make_unique<Board>();
std::shared_ptr<Engine> engine = std::make_shared<Engine>();
```

## File Organization

### Header File Structure

```cpp
#pragma once

// Includes
#include <vector>
#include <string>

// Forward declarations (if needed)
class Move;

// Class declaration
class Board {
public:
    // Constructors
    Board();
    explicit Board(const std::string& fen);
    
    // Public methods
    void makeMove(const Move& move);
    bool isLegalMove(const Move& move) const;
    
private:
    // Private methods
    void updateGameState();
    
    // Member variables
    std::vector<Piece> m_pieces;
    bool m_whiteToMove;
};
```

### Implementation File Structure

```cpp
#include "board/board.h"

#include <stdexcept>
#include <algorithm>

// Constructors
Board::Board() : m_whiteToMove(true) {
    // initialization
}

// Public methods implementation
void Board::makeMove(const Move& move) {
    // implementation
}

// Private methods implementation
void Board::updateGameState() {
    // implementation
}
```

## Code Review Checklist

Before submitting code:

- [ ] Follows naming conventions
- [ ] Properly formatted (use clang-format)
- [ ] Has appropriate comments
- [ ] Includes unit tests
- [ ] No compiler warnings
- [ ] Passes all existing tests
- [ ] Documentation updated if needed

## Tools

### clang-format

Use clang-format for automatic formatting:

```bash
clang-format -i src/**/*.cpp include/**/*.h
```

### clang-tidy

Use clang-tidy for static analysis:

```bash
clang-tidy src/**/*.cpp -- -I include/
```

## References

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
