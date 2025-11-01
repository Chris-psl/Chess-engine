# Chess-engine

## Overview

This repository contains a student chess engine project. The engine implements the core mechanics required to generate and validate chess play and provides a search and evaluation pipeline suitable for experimentation and further development.

## Current status

Implemented
- Board representation and move generation for all piece types.
- Legal move handling including castling, en passant and promotions.
- Basic search algorithms: minimax.
- Simple static evaluation (material + basic positional heuristics).
- Perft-style move-counting utilities for validating the move generator.
- Command-line interface for running self-play and perft tests.

Partially implemented / experimental
- Move ordering heuristics (partial).
- Basic logging and simple CLI options (partial).
- multi-threading(simple).

Not implemented / planned
- Iterative deepening.
- Full UCI protocol support.
- Transposition table and Zobrist hashing.
- Time management and multi-threading.
- Opening book and endgame tablebase integration.
- More advanced evaluation features (mobility, king safety, pawn structure).

## Build & run

Prerequisites
- Typical tools: make or CMake for compiled builds;

Common build steps (adjust to repo files)
- Using Makefile:
    - make
    - ./bin/chess-engine                # or location printed by Makefile

Run perft / tests
  - run the program and select command 1, then input a fen and check the output

If unsure, inspect the top-level files: Makefile, CMakeLists.txt, setup.py, or README snippets in subfolders.

## Example usage

- Play a local game:
    - ./main and select command 2, from there follow the terminal

## Project structure (typical)
- src/        — source code (engine, search, evaluation)
- include/    — public headers (C/C++ projects)
- assets/     — common assets for the gui etc.
- obj/        — built executables


## Roadmap / next steps
- Add transposition table with Zobrist hashing.
- Implement full iterative deepening with time controls.
- Improve evaluation.
- Add UCI protocol support for GUI integration.
