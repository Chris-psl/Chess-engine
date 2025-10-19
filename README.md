# Chess Engine

A chess engine written in C++ from scratch, designed with modularity and expandability in mind.

## Project Status

🚧 **Under Development** - Repository structure established, implementation in progress.

## Features (Planned)

- ♟️ Complete chess rule implementation
- 🧠 Minimax search with alpha-beta pruning
- 📊 Position evaluation engine
- 🔄 UCI (Universal Chess Interface) protocol support
- 🎯 Opening book integration
- ⚡ Performance optimizations (bitboards, transposition tables)
- 🧪 Comprehensive test suite
- 📚 Extensive documentation

## Project Structure

```
Chess-engine/
├── src/              # Source files (.cpp)
│   ├── board/        # Board representation and state
│   ├── moves/        # Move generation and validation
│   ├── evaluation/   # Position evaluation
│   ├── search/       # Search algorithms
│   ├── engine/       # Engine core and UCI protocol
│   └── ui/          # User interface components
├── include/          # Header files (.h)
│   ├── board/
│   ├── moves/
│   ├── evaluation/
│   ├── search/
│   ├── engine/
│   └── ui/
├── tests/            # Test files
│   ├── unit/         # Unit tests
│   └── integration/  # Integration tests
├── examples/         # Example programs
├── docs/            # Documentation
├── tools/           # Build and utility scripts
└── build/           # Build directory (generated)
```

## Getting Started

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or higher
- Git

### Building

#### Quick Start (Unix/Linux/macOS)

```bash
# Clone the repository
git clone https://github.com/Chris-psl/Chess-engine.git
cd Chess-engine

# Build the project
./tools/build.sh
```

#### Manual Build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

For detailed build instructions, see [docs/BUILD.md](docs/BUILD.md).

### Running Tests

```bash
cd build
ctest
```

### Running Examples

```bash
cd build
./basic_game          # Basic game example
./move_generator_demo # Move generation demo
```

## Documentation

- **[Architecture](docs/ARCHITECTURE.md)** - System design and component overview
- **[Build Instructions](docs/BUILD.md)** - Detailed build and setup guide
- **[Contributing](docs/CONTRIBUTING.md)** - Guidelines for contributors
- **[API Documentation](docs/API.md)** - API reference (generated with Doxygen)

## Development Roadmap

### Phase 1: Foundation
- [x] Project structure setup
- [ ] Board representation
- [ ] Basic move generation
- [ ] FEN parser

### Phase 2: Core Engine
- [ ] Legal move generation
- [ ] Position evaluation
- [ ] Search algorithm (minimax + alpha-beta)
- [ ] UCI protocol implementation

### Phase 3: Optimization
- [ ] Transposition tables
- [ ] Move ordering
- [ ] Quiescence search
- [ ] Iterative deepening

### Phase 4: Advanced Features
- [ ] Opening book
- [ ] Endgame tablebases
- [ ] Time management
- [ ] Multi-threading support

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](docs/CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## Architecture Highlights

The engine is designed with these principles:

- **Modularity**: Each component (board, moves, search, evaluation) is independent
- **Testability**: Comprehensive unit and integration tests
- **Expandability**: Easy to add new features without affecting existing code
- **Performance**: Optimized for speed while maintaining code clarity
- **Standards Compliance**: UCI protocol for compatibility with chess GUIs

## Technologies

- **Language**: C++17
- **Build System**: CMake
- **Testing**: Google Test (GTest)
- **Documentation**: Doxygen, Markdown
- **Version Control**: Git

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Chess programming community for algorithms and techniques
- UCI protocol documentation
- Various chess engine projects for inspiration

## Contact

- **Repository**: https://github.com/Chris-psl/Chess-engine
- **Issues**: https://github.com/Chris-psl/Chess-engine/issues

## Resources

- [Chess Programming Wiki](https://www.chessprogramming.org/)
- [UCI Protocol Documentation](http://wbec-ridderkerk.nl/html/UCIProtocol.html)
- [Bitboard Chess Engine in C](https://www.youtube.com/playlist?list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs)

---

**Note**: This is an educational project aimed at understanding chess engine development from first principles.