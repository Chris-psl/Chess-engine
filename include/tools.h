#pragma once
#include <string>
#include <cstdint>
#include <iostream>


// Print a bitboard to the console in 8x8 board format
inline void printBitboard(uint64_t bb) {
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            std::cout << ((bb & (1ULL << sq)) ? "1 " : ". ");
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}