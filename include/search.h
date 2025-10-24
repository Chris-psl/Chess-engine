// search.h - Header file for search algorithms

#pragma once
#include "utils.h"
#include "movegen.h"
#include "evaluate.h"


/**
 * minimax - Implements the Min-Max algorithm to evaluate the best move.
 * @board: Current state of the chess board.
 */
int minimax(BoardState& board, int depth, bool isMaximizingPlayer);