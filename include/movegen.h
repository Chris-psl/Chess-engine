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
 * Helper macros for bitboard manipulation.
 */

#define SET_BIT(bitboard, sq) ((bitboard) |= (1ULL << (sq)))
#define GET_BIT(bitboard, sq) (((bitboard) >> (sq)) & 1ULL)
#define POP_LSB(bb) ({ \
    int lsb = __builtin_ctzll(bb); \
    bb &= bb - 1; \
    lsb; \
})

/**
 * Initializes all precomputed attack tables (knight, king, pawn).
 * Must be called once before generating moves.
 */
void initAttackTables();

/**
 * Generates all pseudo-legal moves for the side to move.
 * Also computes total attack masks for both sides.
 * Most moves are legal, missing en-passant, castling and king exposure checks.
 */
MoveList generateMoves(const BoardState& board);

/**
 * Converts a board index (0..63) to file and rank.
 * Example: squareToCoords(0) -> (a1), 63 -> (h8)
 */
std::string squareToString(int sq);

/**
 * Function that ensures the king is not exposed to check after move generation.
 */
bool isLegalMoveState(const BoardState& board);