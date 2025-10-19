// utils.h
#pragma once
#include <cstdint>
#include <string>

// BoardState struct to hold bitboards and game state
// Bitboards are 64-bit integers representing piece positions for exampl: lsbit corresponds to a1, next bit to b1, ..., msbit to h8.
/*
        a8 b8 c8 d8 e8 f8 g8 h8 <-- msb
        a7 b7 c7 d7 e7 f7 g7 h7
        a6 b6 c6 d6 e6 f6 g6 h6
        a5 b5 c5 d5 e5 f5 g5 h5
        a4 b4 c4 d4 e4 f4 g4 h4
        a3 b3 c3 d3 e3 f3 g3 h3
        a2 b2 c2 d2 e2 f2 g2 h2
lsb --> a1 b1 c1 d1 e1 f1 g1 h1

*/
struct BoardState {
    uint64_t whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing; // Bitboards for white pieces
    uint64_t blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing; // Bitboards for black pieces
    bool whiteToMove;            // true if it's white's turn to move.
    std::string castlingRights;  // e.g., "KQkq" for all castling rights available.
    int enPassantSquare;         // Indicates the square where an en passant capture is possible, -1 if none.
    int halfmoveClock;           // Used for 50-move rule (the game is draw if 50 halfmoves without pawn movement or capture).
    int fullmoveNumber;          // Counts the number of full moves in the game.
};

// Convert BoardState to FEN string
std::string bitboardsToFEN(const BoardState& board);
