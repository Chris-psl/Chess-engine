// search.h - Header file for search algorithms

#pragma once
#include "utils.h"
#include "movegen.h"
#include "evaluate.h"


/**
 * minimax - Implements the Min-Max algorithm to evaluate the best move.
 * @board: Current state of the chess board.
 * @depth: Depth of the search tree.
 * @maximizingPlayer: True for the AI player, false for the opponent.
 */
int minimax(BoardState& board, int depth, bool isMaximizingPlayer);