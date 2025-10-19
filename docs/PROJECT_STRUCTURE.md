# Project Structure

Complete overview of the Chess Engine repository structure.

## Directory Tree

```
Chess-engine/
│
├── .github/                    # GitHub-specific files
│   └── workflows/              # CI/CD workflows
│       └── build.yml           # Automated build and test workflow
│
├── build/                      # Build artifacts (generated, not in git)
│
├── docs/                       # Documentation
│   ├── ARCHITECTURE.md         # System architecture overview
│   ├── BUILD.md                # Detailed build instructions
│   ├── CODING_STYLE.md         # C++ coding standards
│   ├── CONTRIBUTING.md         # Contribution guidelines
│   ├── Doxyfile                # Doxygen configuration
│   ├── QUICK_REFERENCE.md      # Quick reference guide
│   ├── README.md               # Documentation overview
│   └── ROADMAP.md              # Development roadmap
│
├── examples/                   # Example programs
│   ├── CMakeLists.txt          # Examples build configuration
│   └── README.md               # Examples documentation
│   # Future: basic_game.cpp, move_generator_demo.cpp, etc.
│
├── include/                    # Public header files
│   ├── board/                  # Board representation headers
│   │   # Future: board.h, piece.h, square.h
│   ├── engine/                 # Engine core headers
│   │   # Future: engine.h, uci.h, time_manager.h
│   ├── evaluation/             # Evaluation headers
│   │   # Future: evaluator.h, piece_tables.h
│   ├── moves/                  # Move generation headers
│   │   # Future: move.h, move_generator.h, move_list.h
│   ├── search/                 # Search algorithm headers
│   │   # Future: search.h, transposition_table.h
│   ├── ui/                     # UI headers
│   │   # Future: console_ui.h, display.h
│   └── README.md               # Include directory documentation
│
├── src/                        # Implementation files
│   ├── board/                  # Board implementation
│   │   # Future: board.cpp, piece.cpp, square.cpp
│   ├── engine/                 # Engine core implementation
│   │   # Future: engine.cpp, uci.cpp, time_manager.cpp
│   ├── evaluation/             # Evaluation implementation
│   │   # Future: evaluator.cpp, piece_tables.cpp
│   ├── moves/                  # Move generation implementation
│   │   # Future: move.cpp, move_generator.cpp
│   ├── search/                 # Search implementation
│   │   # Future: search.cpp, transposition_table.cpp
│   ├── ui/                     # UI implementation
│   │   # Future: console_ui.cpp, display.cpp
│   └── README.md               # Source directory documentation
│   # Future: main.cpp (entry point)
│
├── tests/                      # Test suite
│   ├── integration/            # Integration tests
│   │   # Future: game_test.cpp, perft_test.cpp
│   ├── unit/                   # Unit tests
│   │   # Future: board_test.cpp, move_generator_test.cpp
│   ├── CMakeLists.txt          # Test build configuration
│   └── README.md               # Testing documentation
│
├── tools/                      # Development scripts
│   ├── build.sh                # Quick build script (Unix)
│   ├── clean.sh                # Clean build artifacts
│   └── README.md               # Tools documentation
│   # Future: run_tests.sh, format_code.sh, lint.sh
│
├── .clang-format               # Code formatting configuration
├── .editorconfig               # Editor configuration
├── .gitignore                  # Git ignore rules
├── CMakeLists.txt              # Main CMake configuration
├── LICENSE                     # MIT License
└── README.md                   # Project overview
```

## File Organization Principles

### Header Files (`include/`)
- **Purpose**: Public API declarations
- **Naming**: `*.h` or `*.hpp`
- **Content**: Class declarations, function prototypes, constants
- **Guidelines**: 
  - Use `#pragma once` for include guards
  - Minimize dependencies
  - Document with Doxygen comments

### Implementation Files (`src/`)
- **Purpose**: Implementation of functionality
- **Naming**: `*.cpp`
- **Content**: Function and method implementations
- **Guidelines**:
  - One .cpp file per .h file
  - Keep files focused and cohesive
  - Limit file size to ~500 lines when possible

### Test Files (`tests/`)
- **Purpose**: Automated testing
- **Naming**: `*_test.cpp`
- **Structure**:
  - `unit/` - Tests for individual components
  - `integration/` - Tests for component interactions
- **Guidelines**:
  - Test edge cases and boundary conditions
  - Aim for high code coverage
  - Use descriptive test names

### Documentation (`docs/`)
- **Purpose**: Project documentation
- **Format**: Markdown (`.md`)
- **Types**:
  - Architecture and design documents
  - Build and setup instructions
  - API documentation (Doxygen)
  - Developer guides

### Examples (`examples/`)
- **Purpose**: Usage demonstrations
- **Naming**: Descriptive names (e.g., `basic_game.cpp`)
- **Content**: Standalone programs showing library usage
- **Guidelines**:
  - Keep examples simple and focused
  - Document with comments
  - Ensure examples stay up-to-date

### Tools (`tools/`)
- **Purpose**: Development and build automation
- **Content**: Shell scripts, batch files
- **Guidelines**:
  - Make scripts executable (`chmod +x`)
  - Add error handling
  - Document usage in README

## Component Dependencies

```
┌─────────────────────────────────────────────┐
│                   main.cpp                   │ (Application Entry Point)
└───────────────────┬─────────────────────────┘
                    │
┌───────────────────▼─────────────────────────┐
│              engine/engine.h                 │ (Engine Controller)
└─┬────────┬──────────┬───────────┬──────────┘
  │        │          │           │
  │        │          │           ├──────────► ui/console_ui.h
  │        │          │           │
  │        │          └──────────► engine/uci.h
  │        │                      
  │        └──────────► search/search.h
  │                    └──┬─────────────┬──────────┐
  │                       │             │          │
  │                       ▼             ▼          ▼
  │                    evaluation/  moves/    board/
  │                    evaluator.h  move.h    board.h
  │                                            │
  └────────────────────────────────────────────┘

All components can access board/ and moves/ modules.
```

## Build Artifacts (Generated)

These directories are created during build and are **not** tracked in git:

```
build/
├── CMakeCache.txt
├── CMakeFiles/
├── Makefile
├── chess_engine              # Main executable
├── libchess_engine_lib.a     # Static library
├── tests/
│   ├── unit_tests            # Unit test executable
│   └── integration_tests     # Integration test executable
└── examples/
    ├── basic_game
    └── move_generator_demo
```

## Key Configuration Files

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Main build configuration |
| `.gitignore` | Files to exclude from git |
| `.clang-format` | Code formatting rules |
| `.editorconfig` | Editor settings |
| `LICENSE` | Project license (MIT) |
| `README.md` | Project overview |

## Documentation Files

| File | Description |
|------|-------------|
| `docs/ARCHITECTURE.md` | System design and components |
| `docs/BUILD.md` | Build instructions |
| `docs/CODING_STYLE.md` | C++ coding standards |
| `docs/CONTRIBUTING.md` | How to contribute |
| `docs/ROADMAP.md` | Development plan |
| `docs/QUICK_REFERENCE.md` | Quick command reference |

## Expandability Design

The structure supports easy expansion:

### Adding New Components
1. Create subdirectories in `src/` and `include/`
2. Update `CMakeLists.txt`
3. Add corresponding tests
4. Update documentation

### Adding New Features
1. Implement in appropriate component directory
2. Add unit tests
3. Add integration tests if needed
4. Document in relevant README

### Adding New Examples
1. Create `.cpp` file in `examples/`
2. Update `examples/CMakeLists.txt`
3. Document in `examples/README.md`

## Next Steps

To start implementing the chess engine:

1. **Start with Phase 1** (Board Representation)
   - Create `include/board/board.h`
   - Create `src/board/board.cpp`
   - Create `tests/unit/board_test.cpp`

2. **Follow the Roadmap** (`docs/ROADMAP.md`)
   - Complete each phase systematically
   - Write tests alongside code
   - Update documentation

3. **Maintain Code Quality**
   - Follow coding style guide
   - Run tests frequently
   - Use CI/CD for validation

## References

- See [ARCHITECTURE.md](ARCHITECTURE.md) for component details
- See [ROADMAP.md](ROADMAP.md) for implementation order
- See [CONTRIBUTING.md](CONTRIBUTING.md) for development workflow
