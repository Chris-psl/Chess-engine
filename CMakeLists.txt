cmake_minimum_required(VERSION 3.14)
project(ChessEngine VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files will be added here as components are implemented
set(CHESS_ENGINE_SOURCES
    # Board representation
    # src/board/board.cpp
    # src/board/piece.cpp
    # src/board/square.cpp
    
    # Move generation
    # src/moves/move_generator.cpp
    # src/moves/move.cpp
    
    # Evaluation
    # src/evaluation/evaluator.cpp
    # src/evaluation/piece_tables.cpp
    
    # Search
    # src/search/search.cpp
    # src/search/minimax.cpp
    # src/search/alpha_beta.cpp
    
    # Engine
    # src/engine/engine.cpp
    # src/engine/uci.cpp
    
    # UI
    # src/ui/console.cpp
)

# Main executable (uncomment when source files are added)
# add_executable(chess_engine ${CHESS_ENGINE_SOURCES} src/main.cpp)

# Library target (uncomment when source files are added)
# add_library(chess_engine_lib STATIC ${CHESS_ENGINE_SOURCES})

# Testing
option(BUILD_TESTS "Build the tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
option(BUILD_EXAMPLES "Build the examples" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation rules
# install(TARGETS chess_engine DESTINATION bin)
# install(DIRECTORY include/ DESTINATION include)
