# Include Directory (include/)

This directory contains all header files (.h or .hpp) with class declarations and interfaces.

## Structure

Each subdirectory mirrors the structure in `src/` and contains:

### board/
- `board.h` - Main board class
- `piece.h` - Piece types and colors
- `square.h` - Square representation
- `bitboard.h` - Bitboard utilities (optional optimization)

### moves/
- `move.h` - Move representation
- `move_generator.h` - Legal move generation
- `move_list.h` - Container for moves

### evaluation/
- `evaluator.h` - Evaluation interface
- `piece_tables.h` - Piece-square tables
- `eval_constants.h` - Evaluation constants and weights

### search/
- `search.h` - Main search interface
- `transposition_table.h` - Hash table for positions
- `search_constants.h` - Search parameters

### engine/
- `engine.h` - Main engine class
- `uci.h` - UCI protocol handler
- `time_manager.h` - Time control management

### ui/
- `console_ui.h` - Console interface
- `display.h` - Display utilities

## Usage

All header files should use include guards or `#pragma once`.
Headers should be included using angle brackets from project root:
```cpp
#include <board/board.h>
#include <moves/move_generator.h>
```
