// Parsing utilities include:
// - FEN string parsing into BoardState with bitboards
// - Conversion from BoardState with bitboards back to FEN string

/*
FEN: rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1 <------------------------ full move number
                                                            /----------------------------- half move clock
*/
#pragma once
#include "tools.h"
#include "utils.h"
#include <string>
#include <sstream>

/**
 * Parses a FEN string and returns the corresponding BoardState with bitboards.
 */
BoardState parseFEN(const std::string& fen);

/**
 * Converts a BoardState with bitboards back to a FEN string.
 */
std::string bitboardsToFEN(const BoardState& board);