#pragma once

#include "utils.h"

/**
 * Evaluates the board state and returns a score.
 * Positive scores favor White, negative scores favor Black.
 * @param board The current board state.
 * @return The evaluation score.
 */
int evaluateBoard(const BoardState& board);