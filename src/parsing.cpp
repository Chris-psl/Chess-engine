// This file contains functions for parsing and creating FEN strings based on the given board state.

#include "utils.h"
#include <sstream>
#include <cctype>
#include <unordered_map>

// Helper to map FEN char to bitboard
inline uint64_t squareMask(int rank, int file) {
    return 1ULL << (rank * 8 + file);
}

// Convert FEN piece character to bitboard reference
void setPieceBitboard(BoardState& state, char c, int rank, int file) {
    uint64_t mask = squareMask(rank, file);
    switch (c) {
        case 'P': state.whitePawns   |= mask; break;
        case 'N': state.whiteKnights |= mask; break;
        case 'B': state.whiteBishops |= mask; break;
        case 'R': state.whiteRooks   |= mask; break;
        case 'Q': state.whiteQueens  |= mask; break;
        case 'K': state.whiteKing    |= mask; break;
        case 'p': state.blackPawns   |= mask; break;
        case 'n': state.blackKnights |= mask; break;
        case 'b': state.blackBishops |= mask; break;
        case 'r': state.blackRooks   |= mask; break;
        case 'q': state.blackQueens  |= mask; break;
        case 'k': state.blackKing    |= mask; break;
    }
}

// Parse FEN string into BoardState with bitboards
BoardState parseFEN(const std::string& fen) {
    
    // Initialize empty board state
    BoardState state{};
    state.whitePawns = state.whiteKnights = state.whiteBishops = state.whiteRooks =
    state.whiteQueens = state.whiteKing = 0;
    state.blackPawns = state.blackKnights = state.blackBishops = state.blackRooks =
    state.blackQueens = state.blackKing = 0;
    state.enPassantSquare = -1;

    std::istringstream iss(fen); // Treats FEN as a stream
    std::string token;

    // 1️⃣ Piece placement
    std::getline(iss, token, ' ');
    int rank = 7; // FEN starts from rank 8
    int file = 0;
    for (char c : token) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (isdigit(c)) {
            file += c - '0'; // empty squares
        } else {
            setPieceBitboard(state, c, rank, file);
            file++;
        }
    }

    // 2️⃣ Active color
    std::getline(iss, token, ' ');
    state.whiteToMove = (token == "w");

    // 3️⃣ Castling rights
    std::getline(iss, token, ' ');
    state.castlingRights = token; // e.g., "KQkq"

    // 4️⃣ En passant target square
    std::getline(iss, token, ' ');
    if (token != "-") {
        char fileChar = token[0]; // a-h
        char rankChar = token[1]; // 1-8
        int epFile = fileChar - 'a';
        int epRank = rankChar - '1';
        state.enPassantSquare = epRank * 8 + epFile;
    }

    // 5️⃣ Halfmove clock
    std::getline(iss, token, ' ');
    state.halfmoveClock = std::stoi(token);

    // 6️⃣ Fullmove number
    std::getline(iss, token, ' ');
    state.fullmoveNumber = std::stoi(token);

    return state;
}
