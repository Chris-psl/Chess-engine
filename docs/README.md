# Documentation Directory

This directory contains documentation for the chess engine project.

## Contents

### Architecture
- `ARCHITECTURE.md` - System design and component overview
- `DESIGN_DECISIONS.md` - Rationale for major design choices

### API Documentation
- `API.md` - Public API reference
- Generated API docs using Doxygen (in html/ after build)

### Development
- `CONTRIBUTING.md` - Guidelines for contributors
- `CODING_STYLE.md` - C++ coding standards for this project
- `BUILD.md` - Detailed build instructions

### Algorithms
- `SEARCH_ALGORITHMS.md` - Search algorithm implementations
- `EVALUATION.md` - Position evaluation details
- `MOVE_GENERATION.md` - Move generation approach

### Protocols
- `UCI_PROTOCOL.md` - UCI implementation details

## Generating Documentation

The project uses Doxygen for API documentation generation.

### Setup Doxygen

```bash
# Install Doxygen
sudo apt-get install doxygen graphviz  # Ubuntu/Debian
brew install doxygen graphviz           # macOS

# Generate documentation
cd docs
doxygen Doxyfile
```

### Viewing Documentation

After generation, open `docs/html/index.html` in your browser.

## Documentation Standards

- Use Markdown for text documentation
- Use Doxygen comments in code for API docs
- Keep documentation up-to-date with code changes
- Include diagrams where helpful (using Mermaid or GraphViz)
