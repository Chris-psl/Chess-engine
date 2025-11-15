// The file uses min-max search algorithm to determine the best move for the current player in a chess game.

#include "search.h"
#include "movegen.h"
#include "evaluate.h"
#include "utils.h"
#include "updateBoard.h"
#include "zobrist.h"
#include "transposition.h"

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

        updateEnPassantSquare(newBoard, move);
        applyMove(newBoard, move);

        int score = -quiescence(newBoard, -beta, -alpha); // negamax-style symmetry

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

// int quiescence(BoardState& board, int alpha, int beta, bool isMaximizingPlayer) {
//     int stand_pat = evaluateBoard(board);

//     if (!isMaximizingPlayer) stand_pat = -stand_pat; // flip evaluation for minimizing

//     if (stand_pat >= beta) return beta;
//     if (alpha < stand_pat) alpha = stand_pat;

//     board.genVolatile = true;
//     MoveList captureMoves = generateLegalMoves(board);
//     board.genVolatile = false;

//     for (const auto& move : captureMoves.moves) {
//         BoardState newBoard = board;
//         applyMove(newBoard, move);

//         int score = quiescence(newBoard, alpha, beta, !isMaximizingPlayer);

//         if (isMaximizingPlayer) {
//             if (score >= beta) return beta;
//             if (score > alpha) alpha = score;
//         } else {
//             if (score <= alpha) return alpha;
//             if (score < beta) beta = score;
//         }
//     }

//     return isMaximizingPlayer ? alpha : beta;
// }


/**
 * minimax - Implements the Min-Max algorithm with Alpha-Beta pruning and transposition tables to evaluate the best move.
 * @board: Current state of the chess board.
 * @depth: Depth of the search tree.
 * @alpha: Alpha value for alpha-beta pruning.
 * @beta: Beta value for alpha-beta pruning.
 * @isMaximizingPlayer: True for the AI player, false for the opponent.
 * The function is an implementation of the Min-Max algorithm with Alpha-Beta pruning and transposition tables.
 */
int minimax(BoardState& board, int depth, int alpha, int beta, bool isMaximizingPlayer) {
    // Compute Zobrist key for this node
    uint64_t key = computeZobristKey(board);

    // Probe transposition table
    TTEntry ttEntry;
    if (TT.probe(key, ttEntry)) {
        if (ttEntry.depth >= depth) {
            // Use stored info according to flag
            if (ttEntry.flag == EXACT) {
                return ttEntry.score;
            } else if (ttEntry.flag == LOWERBOUND) {
                if (ttEntry.score > alpha) alpha = ttEntry.score;
            } else if (ttEntry.flag == UPPERBOUND) {
                if (ttEntry.score < beta) beta = ttEntry.score;
            }
            if (alpha >= beta) {
                return ttEntry.score;
            }
        }
    }

    // Terminal or quiescence
    if (depth == 0) {
        int q = quiescence(board, alpha, beta);
       // int q = quiescence(board, alpha, beta, isMaximizingPlayer);
        // Store Q result into TT as exact at depth 0
        TTEntry storeEntry;
        storeEntry.key = key;
        storeEntry.depth = 0;
        storeEntry.score = q;
        storeEntry.flag = EXACT;
        // bestMove left untouched for quiescence
        TT.store(storeEntry);
        return q;
    }

    MoveList moves = generateLegalMoves(board);

    // If no legal moves (checkmate or stalemate), evaluate board directly
    if (moves.moves.empty()) {
        int ev = evaluateBoard(board);
        // store terminal evaluation
        TTEntry storeEntry;
        storeEntry.key = key;
        storeEntry.depth = depth;
        storeEntry.score = ev;
        storeEntry.flag = EXACT;
        TT.store(storeEntry);
        return ev;
    }

    int originalAlpha = alpha;
    Move bestMoveLocal{};
    int bestScore;

    if (isMaximizingPlayer) {
        bestScore = std::numeric_limits<int>::min();
        for (const auto& move : moves.moves) {
            BoardState newBoard = board;

            updateEnPassantSquare(newBoard, move);
            applyMove(newBoard, move);

            if (!isLegalMoveState(newBoard)) {
                continue; // skip illegal resulting states
            }

            int score = minimax(newBoard, depth - 1, alpha, beta, false);

            if (score > bestScore) {
                bestScore = score;
                bestMoveLocal = move;
            }
            alpha = std::max(alpha, score);
            if (alpha >= beta) {
                break; // beta cutoff
            }
        }
    } else {
        bestScore = std::numeric_limits<int>::max();
        for (const auto& move : moves.moves) {
            BoardState newBoard = board;

            updateEnPassantSquare(newBoard, move);
            applyMove(newBoard, move);

            if (!isLegalMoveState(newBoard)) {
                continue;
            }

            int score = minimax(newBoard, depth - 1, alpha, beta, true);

            if (score < bestScore) {
                bestScore = score;
                bestMoveLocal = move;
            }
            beta = std::min(beta, score);
            if (alpha >= beta) {
                break; // alpha cutoff
            }
        }
    }

    // Determine bound type to store
    BoundType flag = EXACT;
    if (bestScore <= originalAlpha) {
        flag = UPPERBOUND;
    } else if (bestScore >= beta) {
        flag = LOWERBOUND;
    } else {
        flag = EXACT;
    }

    // Store result in TT
    TTEntry storeEntry;
    storeEntry.key = key;
    storeEntry.depth = depth;
    storeEntry.score = bestScore;
    storeEntry.flag = flag;
    storeEntry.bestMove = bestMoveLocal;
    TT.store(storeEntry);

    return bestScore;
}

/**
 * minimax - Implements the Min-Max algorithm to evaluate the best move.
 * @board: Current state of the chess board.
 * @depth: Depth of the search tree.
 * maximizingPlayer: True for the AI player, false for the opponent.
 * The function is an implementation of the Min-Max algorithm with Alpha-Beta pruning.
 * Function call: int bestEval = minimax(board, depth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), true);
 */
// int minimax(BoardState& board, int depth, int alpha, int beta, bool isMaximizingPlayer) {
//     // If reached maximum depth or terminal state, evaluate board
//     if (depth == 0) {
//         //return evaluateBoard(board);
//         return quiescence(board, alpha, beta);
//     }

//     MoveList moves = generateLegalMoves(board);

//     // If no legal moves (checkmate or stalemate), evaluate board directly
//     if (moves.moves.empty()) {
//         return evaluateBoard(board);
//     }

//     if (isMaximizingPlayer) {
//         int maxEval = std::numeric_limits<int>::min();
//         for (const auto& move : moves.moves) {
//             BoardState newBoard = board;
//             applyMove(newBoard, move);

//             int eval = minimax(newBoard, depth - 1, alpha, beta, false);
//             maxEval = std::max(maxEval, eval);
//             alpha = std::max(alpha, eval);

//             // --- Alpha-Beta Pruning ---
//             if (beta <= alpha)
//                 break;
//         }
//         return maxEval;
//     } else {
//         int minEval = std::numeric_limits<int>::max();
//         for (const auto& move : moves.moves) {
//             BoardState newBoard = board;
//             applyMove(newBoard, move);

//             int eval = minimax(newBoard, depth - 1, alpha, beta, true);
//             minEval = std::min(minEval, eval);
//             beta = std::min(beta, eval);

//             // --- Alpha-Beta Pruning ---
//             if (beta <= alpha)
//                 break;
//         }
//         return minEval;
//     }
// }



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
            updateEnPassantSquare(newBoard, move);
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
            updateEnPassantSquare(newBoard, move);
            applyMove(newBoard, move);

            // Recursively call minimax
            int eval = minimax(newBoard, depth - 1, true);
            minEval = std::min(minEval, eval);
        }
        return minEval;
    }
}


// ============================================================================
// with transposition table
// ============================================================================
// int minimax(BoardState& board, int depth, int alpha, int beta, bool isMaximizingPlayer) {
//     extern TranspositionTable TT;  // global TT
//     TTEntry entry;

//     // --- Transposition Table Lookup ---
//     if (TT.probe(board.zobristKey, entry)) {
//         if (entry.depth >= depth) {
//             if (entry.flag == EXACT) return entry.score;
//             if (entry.flag == LOWERBOUND && entry.score >= beta) return entry.score;
//             if (entry.flag == UPPERBOUND && entry.score <= alpha) return entry.score;
//         }
//     }

//     int alphaOrig = alpha; // Save original alpha for flag type later

//     // --- Base Case ---
//     if (depth == 0) {
//         return quiescence(board, alpha, beta); // or evaluateBoard(board)
//     }

//     MoveList moves = generateLegalMoves(board);
//     if (moves.moves.empty()) {
//         return evaluateBoard(board); // checkmate/stalemate
//     }

//     int bestEval;
//     Move bestMove;

//     if (isMaximizingPlayer) {
//         bestEval = std::numeric_limits<int>::min();

//         for (const auto& move : moves.moves) {
//             BoardState newBoard = board;
//             applyMove(newBoard, move);

//             int eval = minimax(newBoard, depth - 1, alpha, beta, false);
//             if (eval > bestEval) {
//                 bestEval = eval;
//                 bestMove = move;
//             }

//             alpha = std::max(alpha, eval);
//             if (beta <= alpha) break; // alpha-beta pruning
//         }

//     } else {
//         bestEval = std::numeric_limits<int>::max();

//         for (const auto& move : moves.moves) {
//             BoardState newBoard = board;
//             applyMove(newBoard, move);

//             int eval = minimax(newBoard, depth - 1, alpha, beta, true);
//             if (eval < bestEval) {
//                 bestEval = eval;
//                 bestMove = move;
//             }

//             beta = std::min(beta, eval);
//             if (beta <= alpha) break; // alpha-beta pruning
//         }
//     }

//     // --- Determine bound type for storage ---
//     BoundType flag;
//     if (bestEval <= alphaOrig) flag = UPPERBOUND;
//     else if (bestEval >= beta) flag = LOWERBOUND;
//     else flag = EXACT;

//     // --- Store in Transposition Table ---
//     TT.store({ board.zobristKey, depth, bestEval, flag, bestMove });

//     return bestEval;
// }
