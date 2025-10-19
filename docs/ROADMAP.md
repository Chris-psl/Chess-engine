# Project Roadmap

This document outlines the development roadmap for the Chess Engine project.

## Current Status: Phase 0 - Foundation Complete ✅

The repository structure is now complete with all necessary scaffolding in place.

---

## Phase 1: Core Board Representation (Estimated: 2-3 weeks)

### 1.1 Basic Data Structures
- [ ] Define Piece enum (Pawn, Knight, Bishop, Rook, Queen, King)
- [ ] Define Color enum (White, Black)
- [ ] Define Square representation (0-63 or 8x8 array)
- [ ] Implement Board class with piece array

### 1.2 Board State Management
- [ ] Castling rights tracking
- [ ] En passant square tracking
- [ ] Half-move clock (for 50-move rule)
- [ ] Full-move counter
- [ ] Side to move

### 1.3 FEN Support
- [ ] FEN string parser
- [ ] FEN string generator
- [ ] Standard starting position constant
- [ ] Validation of FEN strings

### 1.4 Make/Unmake Moves
- [ ] Basic move execution
- [ ] Move reversal (unmake)
- [ ] State history for unmake

### 1.5 Testing
- [ ] Unit tests for board initialization
- [ ] Unit tests for FEN parsing/generation
- [ ] Unit tests for make/unmake moves

---

## Phase 2: Move Generation (Estimated: 3-4 weeks)

### 2.1 Move Representation
- [ ] Move struct (from, to, flags)
- [ ] Special move flags (capture, castling, en passant, promotion)
- [ ] Move list container

### 2.2 Pseudo-Legal Move Generation
- [ ] Pawn moves (single, double push)
- [ ] Pawn captures
- [ ] Knight moves
- [ ] Bishop moves (sliding)
- [ ] Rook moves (sliding)
- [ ] Queen moves (sliding)
- [ ] King moves
- [ ] Castling moves
- [ ] En passant captures
- [ ] Pawn promotion

### 2.3 Legal Move Validation
- [ ] Check detection
- [ ] Pin detection
- [ ] Filter out moves that leave king in check
- [ ] Verify castling legality

### 2.4 Perft Testing
- [ ] Implement perft (performance test)
- [ ] Verify against known perft results
- [ ] Debug move generation issues

### 2.5 Testing
- [ ] Unit tests for each piece type
- [ ] Unit tests for special moves
- [ ] Perft tests for correctness
- [ ] Edge case testing

---

## Phase 3: Position Evaluation (Estimated: 2-3 weeks)

### 3.1 Material Evaluation
- [ ] Piece values (P=100, N=320, B=330, R=500, Q=900)
- [ ] Material counting function

### 3.2 Piece-Square Tables
- [ ] PST for pawns
- [ ] PST for knights
- [ ] PST for bishops
- [ ] PST for rooks
- [ ] PST for queens
- [ ] PST for king (middlegame)
- [ ] PST for king (endgame)

### 3.3 Positional Evaluation
- [ ] Pawn structure evaluation
- [ ] Passed pawns
- [ ] Doubled/isolated pawns
- [ ] King safety
- [ ] Piece mobility
- [ ] Bishop pair bonus

### 3.4 Game Phase Detection
- [ ] Determine game phase (opening/middlegame/endgame)
- [ ] Tapered evaluation

### 3.5 Testing
- [ ] Unit tests for material counting
- [ ] Unit tests for positional features
- [ ] Tactical position tests
- [ ] Endgame position tests

---

## Phase 4: Search Algorithm (Estimated: 4-5 weeks)

### 4.1 Basic Minimax
- [ ] Implement minimax algorithm
- [ ] Fixed-depth search
- [ ] Negamax framework

### 4.2 Alpha-Beta Pruning
- [ ] Alpha-beta pruning
- [ ] Principal variation search
- [ ] Aspiration windows

### 4.3 Move Ordering
- [ ] MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
- [ ] Killer moves
- [ ] History heuristic
- [ ] Hash move ordering

### 4.4 Transposition Table
- [ ] Hash table implementation
- [ ] Zobrist hashing
- [ ] TT entry structure (score, depth, flag, best move)
- [ ] TT replacement strategy

### 4.5 Quiescence Search
- [ ] Capture-only search
- [ ] Standing pat evaluation
- [ ] Delta pruning

### 4.6 Iterative Deepening
- [ ] ID framework
- [ ] Time management integration
- [ ] Depth limits

### 4.7 Testing
- [ ] Tactical puzzle solving
- [ ] Search correctness tests
- [ ] Performance benchmarks
- [ ] Mate-in-N tests

---

## Phase 5: UCI Protocol (Estimated: 2-3 weeks)

### 5.1 Basic UCI Commands
- [ ] `uci` - identify engine
- [ ] `isready` - ready check
- [ ] `ucinewgame` - start new game
- [ ] `position` - set position
- [ ] `go` - start searching
- [ ] `stop` - stop search
- [ ] `quit` - exit engine

### 5.2 Information Output
- [ ] `info` messages
- [ ] Current move
- [ ] Search depth
- [ ] Nodes searched
- [ ] Search speed (nps)
- [ ] Principal variation

### 5.3 Time Management
- [ ] Fixed time per move
- [ ] Fixed depth
- [ ] Time controls (wtime, btime, winc, binc)
- [ ] Nodes limit
- [ ] Infinite search

### 5.4 Options
- [ ] `setoption` command
- [ ] Hash table size option
- [ ] Threads option (for future)
- [ ] Other configurable parameters

### 5.5 Testing
- [ ] UCI protocol compliance tests
- [ ] Integration with chess GUIs
- [ ] Command parsing tests

---

## Phase 6: Optimization & Polish (Estimated: 3-4 weeks)

### 6.1 Performance Optimization
- [ ] Profile code for bottlenecks
- [ ] Optimize move generation
- [ ] Optimize evaluation
- [ ] Consider bitboard representation

### 6.2 Search Enhancements
- [ ] Late move reductions (LMR)
- [ ] Null move pruning
- [ ] Futility pruning
- [ ] Razoring

### 6.3 Evaluation Improvements
- [ ] Tune evaluation weights
- [ ] More advanced features
- [ ] Better endgame handling

### 6.4 Code Quality
- [ ] Code review and refactoring
- [ ] Documentation improvements
- [ ] Performance benchmarks
- [ ] Memory leak checks

### 6.5 Testing
- [ ] Comprehensive test suite
- [ ] Engine vs engine matches
- [ ] Strength testing
- [ ] Stability testing

---

## Phase 7: Advanced Features (Future)

### 7.1 Opening Book
- [ ] Polyglot book format support
- [ ] Book move selection
- [ ] Book learning

### 7.2 Endgame Tablebases
- [ ] Syzygy tablebase probing
- [ ] Tablebase integration in search
- [ ] DTZ (Distance to Zero) handling

### 7.3 Multi-Threading
- [ ] Lazy SMP
- [ ] Thread pool management
- [ ] Parallel search

### 7.4 NNUE (Neural Network Efficiently Updated)
- [ ] NNUE evaluation integration
- [ ] Network training infrastructure
- [ ] Hybrid evaluation

### 7.5 Advanced Search
- [ ] Monte Carlo Tree Search (MCTS)
- [ ] Proof number search
- [ ] Endgame search enhancements

---

## Success Metrics

### Phase 1-3
- ✅ All tests passing
- ✅ Clean code with no memory leaks
- ✅ Perft matches reference values

### Phase 4-5
- 🎯 Solves tactical puzzles (mate in 2-3)
- 🎯 Plays legal chess games
- 🎯 UCI protocol compliant

### Phase 6
- 🎯 Estimated Elo: 1500-1800
- 🎯 Search speed: 100k+ nps
- 🎯 Stable in long games

### Phase 7
- 🎯 Estimated Elo: 2000+
- 🎯 Competitive with established engines
- 🎯 Feature-rich and robust

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute to specific phases.

## Timeline

**Estimated Total Development Time**: 6-12 months for Phases 1-6

The timeline is flexible and depends on:
- Contributor availability
- Code complexity
- Testing thoroughness
- Performance tuning depth

## Notes

- Phases may overlap
- Some tasks may be reordered based on dependencies
- Performance targets are estimates
- Additional features may be added based on interest
