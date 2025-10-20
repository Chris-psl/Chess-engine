#pragma once
#include "utils.h"
#include <vector>
#include <cstdint>
#include <string>

/**
 * Represents a single chess move.
 * - from/to: source and destination squares (0..63)
 * - promotion: piece promoted to ('Q', 'R', 'B', 'N') or '\0' if none
 * - isCapture: true if the move captures a piece
 * - isEnPassant: true if this move is an en passant capture
 * - isCastling: true if this move is a castle
 */
struct Move {
    int from;
    int to;
    char promotion;
    bool isCapture;
    bool isEnPassant;
    bool isCastling;
};

/**
 * Container for all generated moves, along with optional attack masks.
 */
struct MoveList {
    std::vector<Move> moves;
    uint64_t whiteAttacks = 0ULL;
    uint64_t blackAttacks = 0ULL;
};

/**
 * Initializes all precomputed attack tables (knight, king, pawn).
 * Must be called once before generating moves.
 */
void initAttackTables();

/**
 * Generates all pseudo-legal moves for the side to move.
 * Also computes total attack masks for both sides.
 */
MoveList generateMoves(const BoardState& board);
