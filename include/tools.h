// Adds utility functions for printing bitboards and board states

#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include "utils.h"


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

// prints the board with symbols for each piece
// 'P' - white pawn, 'p' - black pawn, 'N' - white knight, 'n' - black knight, etc.
// empty squares are printed as '.'
// with ranks 8 to 1 from top to bottom and files a to h from left to right

inline void printBoard(const BoardState& board) {
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << rank + 1 << " "; // print rank numbers
        for (int file = 0; file < 8; ++file) {
            //std::cout << static_cast<char>('a' + file) << " "; // print file letters
            int sq = rank * 8 + file;
            char pieceChar = '.';
            if (board.whitePawns   & (1ULL << sq)) pieceChar = 'P';
            else if (board.whiteKnights & (1ULL << sq)) pieceChar = 'N';
            else if (board.whiteBishops & (1ULL << sq)) pieceChar = 'B';
            else if (board.whiteRooks   & (1ULL << sq)) pieceChar = 'R';
            else if (board.whiteQueens  & (1ULL << sq)) pieceChar = 'Q';
            else if (board.whiteKing    & (1ULL << sq)) pieceChar = 'K';
            else if (board.blackPawns   & ( 1ULL << sq)) pieceChar = 'p';
            else if (board.blackKnights & (1ULL << sq)) pieceChar = 'n';
            else if (board.blackBishops & (1ULL << sq)) pieceChar = 'b';
            else if (board.blackRooks   & (1ULL << sq)) pieceChar = 'r';
            else if (board.blackQueens  & (1ULL << sq)) pieceChar = 'q';
            else if (board.blackKing    & (1ULL << sq)) pieceChar = 'k';
            std::cout << pieceChar << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}