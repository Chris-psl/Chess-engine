// Chess engine main header file

#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include "utils.h"

/**
 * engine - Main function to process commands and manage the chess engine state.
 * @Command: The command string to be processed (e.g., "uci", "isready", "position", "go").
 * @fenInput: FEN string representing the current board state.
 * @board: Reference to the BoardState structure to be updated or queried.
 * Returns a response string based on the command processed.
 */
std::string engine(std::string Command, std::string fenInput, BoardState& board);

#endif // ENGINE_H