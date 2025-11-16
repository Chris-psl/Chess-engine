// This file contains functions for parsing and creating FEN strings based on the given board state.
// Useful for debugging and testing.

#include "parsing.h"
#include "utils.h"
#include <sstream>
#include <cctype>
#include <unordered_map>

// Helper to map FEN char to bitboard, shifting a 1 to the correct square.
inline uint64_t squareMask(int rank, int file) {
    return 1ULL << (rank * 8 + file);
}

// Convert FEN piece character to bitboard reference, uses bitwise OR to set the piece.
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

    // Piece placement
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

    // Active color
    std::getline(iss, token, ' ');
    state.whiteToMove = (token == "w"); // 'w' for white to move, 'b' for black

    // Castling rights
    std::getline(iss, token, ' ');
    state.castlingRights = token;       // e.g., "KQkq"

    // En passant target square
    std::getline(iss, token, ' ');
    if (token != "-") {
        char fileChar = token[0]; // a-h
        char rankChar = token[1]; // 1-8
        int epFile = fileChar - 'a';
        int epRank = rankChar - '1';
        if(epRank < 0 || epRank > 7 || epFile < 0 || epFile > 7) {
            throw std::invalid_argument("Invalid en passant square in FEN");
        }
        state.enPassantSquare = epRank * 8 + epFile;
    }

    // Halfmove clock
    std::getline(iss, token, ' ');
    state.halfmoveClock = std::stoi(token);

    // Fullmove number
    std::getline(iss, token, ' ');
    state.fullmoveNumber = std::stoi(token);

    return state;
}

// Convert BoardState with bitboards back to FEN string
std::string bitboardsToFEN(const BoardState& board) {
    std::string fen;

    // Piece placement
    for (int rank = 7; rank >= 0; rank--) {
        int emptySquares = 0;
        for (int file = 0; file < 8; file++) {
            uint64_t mask = squareMask(rank, file);
            if (board.whitePawns   & mask) fen += 'P';
            else if (board.whiteKnights & mask) fen += 'N';
            else if (board.whiteBishops & mask) fen += 'B';
            else if (board.whiteRooks   & mask) fen += 'R';
            else if (board.whiteQueens  & mask) fen += 'Q';
            else if (board.whiteKing    & mask) fen += 'K';
            else if (board.blackPawns   & mask) fen += 'p';
            else if (board.blackKnights & mask) fen += 'n';
            else if (board.blackBishops & mask) fen += 'b';
            else if (board.blackRooks   & mask) fen += 'r';
            else if (board.blackQueens  & mask) fen += 'q';
            else if (board.blackKing    & mask) fen += 'k';
            else emptySquares++;
        }
        if (emptySquares > 0) fen += std::to_string(emptySquares);
        if (rank > 0) fen += '/';
    }

    // Active color
    fen += board.whiteToMove ? " w " : " b ";

    // Castling rights
    fen += board.castlingRights + " ";

    // En passant target square
    if (board.enPassantSquare != -1) {
        int file = board.enPassantSquare % 8;
        int rank = board.enPassantSquare / 8;
        fen += std::string(1, 'a' + file) + std::to_string(rank + 1) + " ";
    } else {
        fen += "- ";
    }

    // Halfmove clock
    fen += std::to_string(board.halfmoveClock) + " ";

    // Fullmove number
    fen += std::to_string(board.fullmoveNumber);

    return fen;
}
