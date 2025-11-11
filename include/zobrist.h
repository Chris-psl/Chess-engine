#pragma once
#include <cstdint>
#include "utils.h"

// 12 piece types × 64 squares
extern uint64_t zobristTable[12][64];
extern uint64_t zobristWhiteToMove;
extern uint64_t zobristCastling[16];  // 16 possible combinations of KQkq
extern uint64_t zobristEnPassant[8];  // file a-h
int castlingMask(const std::string& rights);

/**
 * @brief Initializes the Zobrist hashing tables with random values.
 */
void initZobrist();

/**
 * @brief Computes the Zobrist hash key for the given board state.
 * @param board The current board state.
 * @return The computed Zobrist hash key.
 */
uint64_t computeZobristKey(const BoardState& board);
