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
 * quiescence - Extends the search at leaf nodes to avoid horizon effect.
 * @board: Current state of the chess board.
 * @alpha: Alpha value for alpha-beta pruning.
 * @beta: Beta value for alpha-beta pruning.
 * The function explores only capture moves to stabilize the evaluation.
 */
int quiescence(BoardState& board, int alpha, int beta) {
    int stand_pat = evaluateBoard(board);

    // Alpha-beta pruning check
    if (stand_pat >= beta)
        return beta;
    if (alpha < stand_pat)
        alpha = stand_pat;

    // Generate only capture moves (to extend tactical lines)
    board.genVolatile = true;
    MoveList captureMoves = generateLegalMoves(board);
    board.genVolatile = false;

    for (const auto& move : captureMoves.moves) {
        BoardState newBoard = board;
        applyMove(newBoard, move);

        int score = -quiescence(newBoard, -beta, -alpha); // negamax-style symmetry

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}


/**
 * minimax - Implements the Min-Max algorithm to evaluate the best move.
 * @board: Current state of the chess board.
 * @depth: Depth of the search tree.
 * maximizingPlayer: True for the AI player, false for the opponent.
 * The function is an implementation of the Min-Max algorithm with Alpha-Beta pruning.
 * Function call: int bestEval = minimax(board, depth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), true);
 */
int minimax(BoardState& board, int depth, int alpha, int beta, bool isMaximizingPlayer) {
    // If reached maximum depth or terminal state, evaluate board
    if (depth == 0) {
        //return evaluateBoard(board);
        return quiescence(board, alpha, beta);
    }

    MoveList moves = generateLegalMoves(board);

    // If no legal moves (checkmate or stalemate), evaluate board directly
    if (moves.moves.empty()) {
        return evaluateBoard(board);
    }

    if (isMaximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& move : moves.moves) {
            BoardState newBoard = board;
            applyMove(newBoard, move);

            int eval = minimax(newBoard, depth - 1, alpha, beta, false);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);

            // --- Alpha-Beta Pruning ---
            if (beta <= alpha)
                break;
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto& move : moves.moves) {
            BoardState newBoard = board;
            applyMove(newBoard, move);

            int eval = minimax(newBoard, depth - 1, alpha, beta, true);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);

            // --- Alpha-Beta Pruning ---
            if (beta <= alpha)
                break;
        }
        return minEval;
    }
}



/**
 * Regular Min-Max function without alpha-beta pruning.
 * @board: Current state of the chess board.
 * @depth: Depth of the search tree.
 * @isMaximizingPlayer: True for the AI player, false for the opponent.
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