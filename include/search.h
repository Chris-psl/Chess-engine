// search.h - Header file for search algorithms

#pragma once
#include "utils.h"
#include "movegen.h"
#include "evaluate.h"


/**
 * minimax - Implements the Min-Max with alpha-beta pruning algorithm to evaluate the best move.
 * @board: Current state of the chess board.
 * @depth: Depth of the search tree.
 * @maximizingPlayer: True for the AI player, false for the opponent.
 */
int minimax(BoardState& board, int depth, int alpha, int beta, bool isMaximizingPlayer);

/**
 * Regular Min-Max function without alpha-beta pruning.
 * @board: Current state of the chess board.
 * @depth: Depth of the search tree.
 * @isMaximizingPlayer: True for the AI player, false for the opponent.
 */
int minimax(BoardState& board, int depth, bool isMaximizingPlayer);