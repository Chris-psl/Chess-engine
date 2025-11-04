// The file uses min-max search algorithm to determine the best move for the current player in a chess game.

#include "search.h"
#include "movegen.h"
#include "evaluate.h"
#include "utils.h"
#include "updateBoard.h"

#include <vector>
#include <limits>
#include <iostream>


// ============================================================================
//  SECTION 1: MIN-MAX SEARCH ALGORITHM
// ============================================================================


/**
 * minimax - Implements the Min-Max algorithm to evaluate the best move.
 * @board: Current state of the chess board.
 * @depth: Depth of the search tree.
 * maximizingPlayer: True for the AI player, false for the opponent.
 */
int minimax(BoardState& board, int depth, bool isMaximizingPlayer) {
    // Generate all possible moves for the current player
    //initAttackTables();

    // if reached the maximum depth, evaluate the board
    if (depth == 0) {
        return evaluateBoard(board);
    }

    MoveList moves = generateLegalMoves(board);
    
    if (isMaximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& move : moves.moves) {
            // Create a copy of the board
            BoardState newBoard = board;

            // Apply move to newBoard
            applyMove(newBoard, move);

            // Recursively call minimax
            int eval = minimax(newBoard, depth - 1, false);
            maxEval = std::max(maxEval, eval);
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto& move : moves.moves) {
            // Create a copy of the board
            BoardState newBoard = board; 

            // Apply move to newBoard
            applyMove(newBoard, move);

            // Recursively call minimax
            int eval = minimax(newBoard, depth - 1, true);
            minEval = std::min(minEval, eval);
        }
        return minEval;
    }
}