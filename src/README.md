# Source Directory (src/)

This directory contains all the implementation files (.cpp) for the chess engine.

## Structure

### board/
Board representation and state management:
- Board data structure
- Piece definitions and properties
- Square/position handling
- FEN (Forsyth-Edwards Notation) parsing and generation
- Board state (castling rights, en passant, etc.)

### moves/
Move generation and validation:
- Legal move generation
- Move representation
- Move validation
- Special moves (castling, en passant, promotion)
- Move ordering for search optimization

### evaluation/
Position evaluation functions:
- Static evaluation
- Piece-square tables
- Material counting
- Positional evaluation
- Endgame evaluation
- Pawn structure analysis

### search/
Search algorithms:
- Minimax algorithm
- Alpha-beta pruning
- Iterative deepening
- Transposition tables
- Quiescence search
- Move ordering

### engine/
Engine core and communication:
- Main engine logic
- UCI (Universal Chess Interface) protocol implementation
- Time management
- Opening book handling
- Endgame tablebase integration (future)

### ui/
User interface components:
- Console-based interface
- Board visualization
- Move input/output
- Game state display
- Debug utilities

## Building

Each subdirectory will contain .cpp files that correspond to headers in the `include/` directory.
