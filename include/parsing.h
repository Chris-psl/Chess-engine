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

// Parse FEN string into BoardState with bitboards
BoardState parseFEN(const std::string& fen);

// Convert BoardState with bitboards back to FEN string
std::string bitboardsToFEN(const BoardState& board);