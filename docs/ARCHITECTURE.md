# Chess Engine Architecture

## Overview

This chess engine is designed with modularity and expandability in mind. Each component has a specific responsibility and communicates through well-defined interfaces.

## Component Overview

```
┌─────────────────────────────────────────────┐
│              UCI Interface                   │
│         (Communication Protocol)             │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────▼───────────────────────────┐
│              Engine Core                     │
│      (Orchestrates all components)           │
└──┬──────────┬──────────┬───────────┬────────┘
   │          │          │           │
   ▼          ▼          ▼           ▼
┌──────┐  ┌──────┐  ┌─────────┐  ┌──────┐
│Board │  │Search│  │Evaluation│  │ UI   │
│      │  │      │  │          │  │      │
└──┬───┘  └──┬───┘  └────┬─────┘  └──────┘
   │         │           │
   ▼         │           │
┌──────────┐ │           │
│   Moves  │◄┘           │
│Generator │◄────────────┘
└──────────┘
```

## Core Components

### 1. Board Representation
**Location:** `src/board/`, `include/board/`

Manages the chess board state:
- Piece positions
- Game state (castling rights, en passant, half-move clock)
- FEN string parsing and generation
- Make/unmake move functionality

**Key Classes:**
- `Board` - Main board representation
- `Piece` - Piece types and properties
- `Square` - Board square representation

### 2. Move Generation
**Location:** `src/moves/`, `include/moves/`

Generates legal moves:
- Pseudo-legal move generation
- Legal move validation
- Special moves handling
- Move ordering for search optimization

**Key Classes:**
- `MoveGenerator` - Generates all legal moves
- `Move` - Represents a chess move
- `MoveList` - Container for moves

### 3. Position Evaluation
**Location:** `src/evaluation/`, `include/evaluation/`

Evaluates chess positions:
- Material balance
- Piece-square tables
- Positional features (pawn structure, king safety, etc.)
- Endgame-specific evaluation

**Key Classes:**
- `Evaluator` - Main evaluation engine
- `PieceTables` - Position-dependent piece values

### 4. Search
**Location:** `src/search/`, `include/search/`

Implements search algorithms:
- Minimax with alpha-beta pruning
- Iterative deepening
- Transposition tables
- Quiescence search
- Move ordering

**Key Classes:**
- `Search` - Main search controller
- `TranspositionTable` - Position caching

### 5. Engine Core
**Location:** `src/engine/`, `include/engine/`

Coordinates all components:
- Time management
- Search control
- Opening book (future)
- Endgame tablebases (future)

**Key Classes:**
- `Engine` - Main engine controller
- `UCI` - Universal Chess Interface implementation
- `TimeManager` - Time control management

### 6. User Interface
**Location:** `src/ui/`, `include/ui/`

Provides interaction capabilities:
- Console-based UI
- Board display
- Move input/output
- Debug information display

**Key Classes:**
- `ConsoleUI` - Text-based interface
- `Display` - Board visualization

## Data Flow

1. **User Input** → UCI Interface → Engine Core
2. **Engine Core** → Board (current position)
3. **Engine Core** → Search (find best move)
4. **Search** → Move Generator (get legal moves)
5. **Search** → Evaluator (evaluate positions)
6. **Search** → Engine Core (return best move)
7. **Engine Core** → UCI Interface → User Output

## Extensibility Points

### Adding New Features

1. **New Search Technique**
   - Implement in `src/search/`
   - Update `Search` class interface
   - No changes needed to other components

2. **Enhanced Evaluation**
   - Add new evaluation terms in `src/evaluation/`
   - Update `Evaluator` class
   - Search and Engine components unchanged

3. **Alternative Board Representation**
   - Implement new representation in `src/board/`
   - Maintain same interface
   - All other components continue working

4. **New UI Type (GUI)**
   - Create new directory `src/ui/gui/`
   - Implement UI interface
   - No changes to engine components

## Design Principles

1. **Separation of Concerns**: Each component has one primary responsibility
2. **Interface-Based Design**: Components communicate through interfaces
3. **Loose Coupling**: Minimize dependencies between components
4. **High Cohesion**: Related functionality grouped together
5. **Testability**: Each component can be tested independently

## Performance Considerations

- Move generation is performance-critical
- Consider bitboard representation for speed
- Transposition tables for search efficiency
- Profile before optimizing

## Future Enhancements

- Opening book support
- Endgame tablebase integration
- Neural network evaluation (NNUE)
- Multi-threaded search
- Pondering (thinking on opponent's time)
