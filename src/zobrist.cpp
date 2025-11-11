#include "zobrist.h"
#include <random>
#include <string>

// global zobrist arrays
uint64_t zobristTable[12][64];
uint64_t zobristWhiteToMove;
uint64_t zobristCastling[16];
uint64_t zobristEnPassant[8];

static uint64_t random64() {
    static std::mt19937_64 rng(2025);  // fixed seed for determinism
    static std::uniform_int_distribution<uint64_t> dist;
    return dist(rng);
}

void initZobrist() {
    // Fill piece-square table
    for (int p = 0; p < 12; ++p)
        for (int sq = 0; sq < 64; ++sq)
            zobristTable[p][sq] = random64();

    zobristWhiteToMove = random64();

    // 16 combinations of KQkq castling rights
    for (int i = 0; i < 16; ++i)
        zobristCastling[i] = random64();

    // En passant for each file
    for (int i = 0; i < 8; ++i)
        zobristEnPassant[i] = random64();
}

// Helper: convert castling string (like "KQkq") into a 4-bit mask
int castlingMask(const std::string& rights) {
    int mask = 0;
    if (rights.find('K') != std::string::npos) mask |= 1 << 0;
    if (rights.find('Q') != std::string::npos) mask |= 1 << 1;
    if (rights.find('k') != std::string::npos) mask |= 1 << 2;
    if (rights.find('q') != std::string::npos) mask |= 1 << 3;
    return mask;
}

uint64_t computeZobristKey(const BoardState& board) {
    uint64_t key = 0;

    auto addPieces = [&](uint64_t bitboard, int pieceIndex) {
        while (bitboard) {
            int sq = __builtin_ctzll(bitboard);  // least significant bit index
            key ^= zobristTable[pieceIndex][sq];
            bitboard &= bitboard - 1; // clear lsbit
        }
    };

    // Add all piece positions
    addPieces(board.whitePawns,   0);
    addPieces(board.whiteKnights, 1);
    addPieces(board.whiteBishops, 2);
    addPieces(board.whiteRooks,   3);
    addPieces(board.whiteQueens,  4);
    addPieces(board.whiteKing,    5);
    addPieces(board.blackPawns,   6);
    addPieces(board.blackKnights, 7);
    addPieces(board.blackBishops, 8);
    addPieces(board.blackRooks,   9);
    addPieces(board.blackQueens, 10);
    addPieces(board.blackKing,   11);

    // Side to move
    if (!board.whiteToMove)
        key ^= zobristWhiteToMove;

    // Castling rights
    int mask = castlingMask(board.castlingRights);
    key ^= zobristCastling[mask];

    // En passant (only if valid)
    if (board.enPassantSquare != -1) {
        int file = board.enPassantSquare % 8;
        key ^= zobristEnPassant[file];
    }

    return key;
}
