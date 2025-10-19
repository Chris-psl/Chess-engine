#pragma once
#include <iostream>
#include <bitset>
#include "utils.h"

// Define this macro in your project or before including the header
#ifndef DEBUG_BITBOARD
#define DEBUG_BITBOARD false
#endif

struct BoardState {
    uint64_t whitePawns = 0;
    uint64_t whiteKnights = 0;
    uint64_t whiteBishops = 0;
    uint64_t whiteRooks = 0;
    uint64_t whiteQueens = 0;
    uint64_t whiteKing = 0;
    uint64_t blackPawns = 0;
    uint64_t blackKnights = 0;
    uint64_t blackBishops = 0;
    uint64_t blackRooks = 0;
    uint64_t blackQueens = 0;
    uint64_t blackKing = 0;
};

// Conditional debug print function
#if DEBUG_BITBOARD
inline void printBitboard(uint64_t board) {
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            uint64_t mask = 1ULL << (rank * 8 + file);
            std::cout << ((board & mask) ? "1 " : ". ");
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

inline void printBoardState(const BoardState& state) {
    std::cout << "White Pawns:\n"; printBitboard(state.whitePawns);
    std::cout << "White Knights:\n"; printBitboard(state.whiteKnights);
    std::cout << "White Bishops:\n"; printBitboard(state.whiteBishops);
    std::cout << "White Rooks:\n"; printBitboard(state.whiteRooks);
    std::cout << "White Queens:\n"; printBitboard(state.whiteQueens);
    std::cout << "White King:\n"; printBitboard(state.whiteKing);
    std::cout << "Black Pawns:\n"; printBitboard(state.blackPawns);
    std::cout << "Black Knights:\n"; printBitboard(state.blackKnights);
    std::cout << "Black Bishops:\n"; printBitboard(state.blackBishops);
    std::cout << "Black Rooks:\n"; printBitboard(state.blackRooks);
    std::cout << "Black Queens:\n"; printBitboard(state.blackQueens);
    std::cout << "Black King:\n"; printBitboard(state.blackKing);
}
#else
// If DEBUG_BITBOARD is false, define empty functions
inline void printBitboard(uint64_t) {}
inline void printBoardState(const BoardState&) {}
#endif

// General debug print function
#ifndef debug
#define debug false
#endif

#if debug
#include <iostream>
inline void debugPrint(const std::string& message) {
    std::cout << "[DEBUG] " << message << std::endl;
}
#else

inline void debugPrint(const std::string&) {}
#endif