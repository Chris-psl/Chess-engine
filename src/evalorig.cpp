// This is a file that contains functions for evaluating a chess board state.

#include "evaluate.h"
#include "movegen.h"

#include <cstdint>
#include <cassert>
#include <array>
#include <iostream>


enum GamePhase { OPENING, MIDGAME, ENDGAME };

GamePhase determine_game_phase(const BoardState& board) {
    // Piece weights
    constexpr int QUEEN_WEIGHT  = 9;
    constexpr int ROOK_WEIGHT   = 5;
    constexpr int BISHOP_WEIGHT = 3;
    constexpr int KNIGHT_WEIGHT = 3;
    constexpr int PAWN_WEIGHT   = 1;

    // === Material calculation ===
    int totalMaterial =
        (__builtin_popcountll(board.whiteQueens) + __builtin_popcountll(board.blackQueens)) * QUEEN_WEIGHT +
        (__builtin_popcountll(board.whiteRooks)  + __builtin_popcountll(board.blackRooks))  * ROOK_WEIGHT +
        (__builtin_popcountll(board.whiteBishops) + __builtin_popcountll(board.blackBishops)) * BISHOP_WEIGHT +
        (__builtin_popcountll(board.whiteKnights) + __builtin_popcountll(board.blackKnights)) * KNIGHT_WEIGHT +
        (__builtin_popcountll(board.whitePawns)  + __builtin_popcountll(board.blackPawns))  * PAWN_WEIGHT;

    constexpr int STARTING_MATERIAL = 
        2*QUEEN_WEIGHT + 4*ROOK_WEIGHT + 4*BISHOP_WEIGHT + 4*KNIGHT_WEIGHT + 16*PAWN_WEIGHT;

    double materialRatio = static_cast<double>(totalMaterial) / STARTING_MATERIAL;

    // === Move factor ===
    double moveFactor = 1.0;
    if (board.fullmoveNumber > 20)
        moveFactor -= std::min(0.4, (board.fullmoveNumber - 20) * 0.02);

    // === Pawn activity factor ===
    int whitePawns = __builtin_popcountll(board.whitePawns);
    int blackPawns = __builtin_popcountll(board.blackPawns);
    int pawnDiff   = std::abs(whitePawns - blackPawns);

    // Advanced pawns: ranks 5-7 for white, 2-4 for black
    uint64_t advancedWhite = board.whitePawns & 0x00FFFFFF00000000ULL;
    uint64_t advancedBlack = board.blackPawns & 0x000000FFFFFF0000ULL;
    int pawnActivity = __builtin_popcountll(advancedWhite) + __builtin_popcountll(advancedBlack);

    double pawnActivityFactor = 1.0 - std::min(0.3, (pawnDiff + pawnActivity) * 0.03);

    // === Mobility factor (rough estimate) ===
    int estimatedMobility = 64 - __builtin_popcountll(board.whitePawns | board.whiteKnights |
                                                     board.whiteBishops | board.whiteRooks | board.whiteQueens) +
                            64 - __builtin_popcountll(board.blackPawns | board.blackKnights |
                                                     board.blackBishops | board.blackRooks | board.blackQueens);
    double mobilityFactor = std::min(1.0, static_cast<double>(estimatedMobility) / 128.0);

    // === Open files factor ===
    uint64_t allPawns = board.whitePawns | board.blackPawns;
    int openFiles = 0;
    for (int file = 0; file < 8; ++file) {
        uint64_t fileMask = 0x0101010101010101ULL << file;
        if ((allPawns & fileMask) == 0) openFiles++;
    }
    double openFileFactor = static_cast<double>(openFiles) / 8.0;

    // === Queen presence factor ===
    int numQueens = __builtin_popcountll(board.whiteQueens | board.blackQueens);
    double queenFactor = numQueens > 0 ? 1.0 : 0.5;

    // === Combine factors ===
    double phaseScore =
        (materialRatio      * 0.50) +
        (moveFactor         * 0.20) +
        (pawnActivityFactor * 0.10) +
        (mobilityFactor     * 0.10) +
        (openFileFactor     * 0.05) +
        (queenFactor        * 0.05);

    // === Classify phase ===
    if (phaseScore > 0.7)
        return OPENING;
    else if (phaseScore > 0.4)
        return MIDGAME;
    else
        return ENDGAME;
}


// Material-based evaluation function
int material_score(const BoardState& board) {
    const int PAWN_VALUE   = 100;
    const int KNIGHT_VALUE = 320;
    const int BISHOP_VALUE = 330;
    const int ROOK_VALUE   = 500;
    const int QUEEN_VALUE  = 900;
    const int BISHOP_PAIR  = 30;
    const int TEMPO_BONUS  = 10;

    int whiteScore = 0;
    whiteScore += __builtin_popcountll(board.whitePawns)   * PAWN_VALUE;
    whiteScore += __builtin_popcountll(board.whiteKnights) * KNIGHT_VALUE;
    whiteScore += __builtin_popcountll(board.whiteBishops) * BISHOP_VALUE;
    whiteScore += __builtin_popcountll(board.whiteRooks)   * ROOK_VALUE;
    whiteScore += __builtin_popcountll(board.whiteQueens)  * QUEEN_VALUE;
    if (__builtin_popcountll(board.whiteBishops) >= 2) whiteScore += BISHOP_PAIR;

    int blackScore = 0;
    blackScore += __builtin_popcountll(board.blackPawns)   * PAWN_VALUE;
    blackScore += __builtin_popcountll(board.blackKnights) * KNIGHT_VALUE;
    blackScore += __builtin_popcountll(board.blackBishops) * BISHOP_VALUE;
    blackScore += __builtin_popcountll(board.blackRooks)   * ROOK_VALUE;
    blackScore += __builtin_popcountll(board.blackQueens)  * QUEEN_VALUE;
    if (__builtin_popcountll(board.blackBishops) >= 2) blackScore += BISHOP_PAIR;

    int score = whiteScore - blackScore;
    if (board.whiteToMove) score += TEMPO_BONUS;

    return score;
}


///////////// Piece-Square Tables /////////////


// Helper: pop LSB index and clear it (lsb = a1 convention)
static inline int pop_lsb_index(uint64_t &bb) {
    assert(bb != 0);
    int idx = __builtin_ctzll(bb);   // count trailing zeros => index of LSB
    bb &= bb - 1;                    // clear LSB
    return idx;
}

// Mirror square vertically (rank flip) for LSB=a1 mapping.
// Example: a1(0) -> a8(56), h1(7)->h8(63), a2(8)->a7(48)
static inline int mirror_vertical(int sq) {
    return sq ^ 56;
}

// Evaluate bitboard for white side using table (table indexed with a1=0 ... h8=63)
static inline int eval_bitboard(const uint64_t bb_in, const int table[64]) {
    uint64_t bb = bb_in;
    int sum = 0;
    while (bb) {
        int sq = pop_lsb_index(bb);
        sum += table[sq];
    }
    return sum;
}

// Evaluate bitboard for black side: mirror the square vertically before indexing
static inline int eval_bitboard_black(const uint64_t bb_in, const int table[64]) {
    uint64_t bb = bb_in;
    int sum = 0;
    while (bb) {
        int sq = pop_lsb_index(bb);
        sum += table[mirror_vertical(sq)];
    }
    return sum;
}

// Main function: piece-square table based evaluation
int piece_square_table_score(const BoardState& board, GamePhase phase) {
    // Tables are defined from white's perspective with index 0 = a1, 63 = h8.
    // (Using the values you provided; keep or tweak them as you like.)

    // --------- Opening tables ----------
    static const int pawn_open[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 27, 27, 10,  5,  5,
        0,  0,  0, 25, 25,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-25,-25, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    static const int knight_open[64] = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-20,-30,-30,-20,-40,-50
    };

    static const int rook_open[64] = {
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        10, 10, 10, 10, 10, 10, 10, 10,
         0,  0,  0,  5,  5,  0,  0,  0
    };

    static const int bishop_open[64] = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-40,-10,-10,-40,-10,-20
    };

    static const int queen_open[64] = {
        -5, -5, -5, -2, -2, -5, -5, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -2,  0,  5, 10, 10,  5,  0, -2,
        -2,  0,  5, 10, 10,  5,  0, -2,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5, -5, -5, -2, -2, -5, -5, -5
    };

    static const int king_open[64] = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 20,  0,  0,  0,  0, 20, 20,
         20, 30, 10,  0,  0, 10, 30, 20
    };

    // --------- Midgame tables (using same values as opening in your sample) ----------
    static const int pawn_mid[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 27, 27, 10,  5,  5,
        0,  0,  0, 25, 25,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-25,-25, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    };
    static const int knight_mid[64] = { /* identical to knight_open in your input */ 
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-20,-30,-30,-20,-40,-50
    };
    static const int rook_mid[64] = { /* same as rook_open */ 
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        10, 10, 10, 10, 10, 10, 10, 10,
        0,  0,  0,  5,  5,  0,  0,  0
    };
    static const int bishop_mid[64] = { /* same as bishop_open */ 
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-40,-10,-10,-40,-10,-20
    };
    static const int queen_mid[64] = { /* same as queen_open */ 
        -5, -5, -5, -2, -2, -5, -5, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -2,  0,  5, 10, 10,  5,  0, -2,
        -2,  0,  5, 10, 10,  5,  0, -2,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5, -5, -5, -2, -2, -5, -5, -5
    };
    static const int king_mid[64] = { /* same as king_open */ 
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 20,  0,  0,  0,  0, 20, 20,
         20, 30, 10,  0,  0, 10, 30, 20
    };

    // --------- Endgame tables (only a few differ in your sample; included here) ----------
    static const int pawn_end[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 27, 27, 10,  5,  5,
        0,  0,  0, 25, 25,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        10, 10, 10,-25,-25, 10, 10, 10,
        0,  0,  0,  0,  0,  0,  0,  0
    };
    static const int king_end[64] = {
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-40,-30,-20,-20,-30,-40,-50
    };
    static const int knight_end[64] = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-20,-30,-30,-20,-40,-50
    };
    static const int rook_end[64] = {
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        10, 10, 10, 10, 10, 10, 10, 10,
         0,  0,  0,  5,  5,  0,  0,  0
    };
    static const int bishop_end[64] = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-40,-10,-10,-40,-10,-20
    };
    static const int queen_end[64] = {
        -5, -5, -5, -2, -2, -5, -5, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -2,  0,  5, 10, 10,  5,  0, -2,
        -2,  0,  5, 10, 10,  5,  0, -2,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5, -5, -5, -2, -2, -5, -5, -5
    };

    // Sum up according to phase
    int score = 0;
    if (phase == OPENING) {
        score += eval_bitboard(board.whitePawns, pawn_open);
        score -= eval_bitboard_black(board.blackPawns, pawn_open);

        score += eval_bitboard(board.whiteKnights, knight_open);
        score -= eval_bitboard_black(board.blackKnights, knight_open);

        score += eval_bitboard(board.whiteBishops, bishop_open);
        score -= eval_bitboard_black(board.blackBishops, bishop_open);

        score += eval_bitboard(board.whiteRooks, rook_open);
        score -= eval_bitboard_black(board.blackRooks, rook_open);

        score += eval_bitboard(board.whiteQueens, queen_open);
        score -= eval_bitboard_black(board.blackQueens, queen_open);

        score += eval_bitboard(board.whiteKing, king_open);
        score -= eval_bitboard_black(board.blackKing, king_open);

        return score;
    }
    else if (phase == MIDGAME) {
        score += eval_bitboard(board.whitePawns, pawn_mid);
        score -= eval_bitboard_black(board.blackPawns, pawn_mid);

        score += eval_bitboard(board.whiteKnights, knight_mid);
        score -= eval_bitboard_black(board.blackKnights, knight_mid);

        score += eval_bitboard(board.whiteBishops, bishop_mid);
        score -= eval_bitboard_black(board.blackBishops, bishop_mid);

        score += eval_bitboard(board.whiteRooks, rook_mid);
        score -= eval_bitboard_black(board.blackRooks, rook_mid);

        score += eval_bitboard(board.whiteQueens, queen_mid);
        score -= eval_bitboard_black(board.blackQueens, queen_mid);

        score += eval_bitboard(board.whiteKing, king_mid);
        score -= eval_bitboard_black(board.blackKing, king_mid);

        return score;
    }
    else { // ENDGAME
        score += eval_bitboard(board.whitePawns, pawn_end);
        score -= eval_bitboard_black(board.blackPawns, pawn_end);

        score += eval_bitboard(board.whiteKnights, knight_end);
        score -= eval_bitboard_black(board.blackKnights, knight_end);

        score += eval_bitboard(board.whiteBishops, bishop_end);
        score -= eval_bitboard_black(board.blackBishops, bishop_end);

        score += eval_bitboard(board.whiteRooks, rook_end);
        score -= eval_bitboard_black(board.blackRooks, rook_end);

        score += eval_bitboard(board.whiteQueens, queen_end);
        score -= eval_bitboard_black(board.blackQueens, queen_end);

        score += eval_bitboard(board.whiteKing, king_end);
        score -= eval_bitboard_black(board.blackKing, king_end);

        return score;
    }
}

///////////// Pawn Structure Evaluation /////////////
int pawn_structure_score(const BoardState& board, GamePhase phase) {
    int doubledPawnPenalty = -10;
    int isolatedPawnPenalty = -8;
    int passedPawnBonus = 15;
    int backwardPawnPenalty = -6;
    int pawnChainBonus = 5;

    // Προσαρμογή weights ανάλογα με το στάδιο
    switch(phase) {
        case OPENING:
            doubledPawnPenalty /= 2;
            isolatedPawnPenalty /= 2;
            passedPawnBonus /= 2;
            backwardPawnPenalty /= 2;
            pawnChainBonus /= 2;
            break;
        case ENDGAME:
            doubledPawnPenalty /= 2;
            isolatedPawnPenalty /= 2;
            passedPawnBonus *= 2; // τα περασμένα πιόνια πολύ πιο σημαντικά
            backwardPawnPenalty *= 2;
            break;
        default:
            break; // midgame πλήρη weights
    }
  int whiteScore = 0;
    int blackScore = 0;

    uint64_t whitePawns = board.whitePawns;
    uint64_t blackPawns = board.blackPawns;

    // --- Εξετάζουμε κάθε στήλη (file a–h) ---
    for (int file = 0; file < 8; ++file) {
        uint64_t fileMask = 0x0101010101010101ULL << file;

        // Πόσα πιόνια έχει κάθε πλευρά στη συγκεκριμένη στήλη
        int whiteOnFile = __builtin_popcountll(whitePawns & fileMask);
        int blackOnFile = __builtin_popcountll(blackPawns & fileMask);

        // --- Διπλωμένα πιόνια ---
        if (whiteOnFile > 1)
            whiteScore += doubledPawnPenalty * (whiteOnFile - 1);
        if (blackOnFile > 1)
            blackScore += doubledPawnPenalty * (blackOnFile - 1);

        // --- Απομονωμένα πιόνια ---
        uint64_t adjacentFiles = 0;
        if (file > 0) adjacentFiles |= 0x0101010101010101ULL << (file - 1);
        if (file < 7) adjacentFiles |= 0x0101010101010101ULL << (file + 1);

        bool whiteIsolated = (whitePawns & fileMask) &&
                             !(whitePawns & adjacentFiles);
        bool blackIsolated = (blackPawns & fileMask) &&
                             !(blackPawns & adjacentFiles);

        if (whiteIsolated) whiteScore += isolatedPawnPenalty;
        if (blackIsolated) blackScore += isolatedPawnPenalty;

        // --- Περασμένα πιόνια ---
        uint64_t whiteBlockers = (blackPawns &
            ((file > 0 ? 0x0101010101010101ULL << (file - 1) : 0) |
             fileMask |
             (file < 7 ? 0x0101010101010101ULL << (file + 1) : 0)));

        uint64_t blackBlockers = (whitePawns &
            ((file > 0 ? 0x0101010101010101ULL << (file - 1) : 0) |
             fileMask |
             (file < 7 ? 0x0101010101010101ULL << (file + 1) : 0)));

        uint64_t w = whitePawns & fileMask;
        while (w) {
            int sq = __builtin_ctzll(w);
            uint64_t maskAhead = (~0ULL) << (sq + 8);
            if (!(whiteBlockers & maskAhead))
                whiteScore += passedPawnBonus;
            w &= w - 1;
        }

        uint64_t b = blackPawns & fileMask;
        while (b) {
            int sq = __builtin_ctzll(b);
            uint64_t maskAhead = ((1ULL << sq) - 1);
            if (!(blackBlockers & maskAhead))
                blackScore += passedPawnBonus;
            b &= b - 1;
        }
    }

    // --- Ανάλυση ανά πιόνι για chains & backward pawns ---
    uint64_t wp = whitePawns;
    while (wp) {
        int sq = __builtin_ctzll(wp);
        wp &= wp - 1;

        uint64_t bit = 1ULL << sq;

        // --- Pawn chain: υποστήριξη διαγώνια από άλλο πιόνι ---
        uint64_t supportingPawns = ((bit >> 7) | (bit >> 9)) & whitePawns;
        if (supportingPawns)
            whiteScore += pawnChainBonus;

        // --- Backward pawn: δεν έχει υποστήριξη και δεν μπορεί να προχωρήσει ---
        uint64_t front = bit << 8; // τετράγωνο μπροστά
        uint64_t leftSupport = bit >> 9;
        uint64_t rightSupport = bit >> 7;
        if (!(whitePawns & (leftSupport | rightSupport)) && !(front & ~whitePawns))
            whiteScore += backwardPawnPenalty;
    }

    uint64_t bp = blackPawns;
    while (bp) {
        int sq = __builtin_ctzll(bp);
        bp &= bp - 1;

        uint64_t bit = 1ULL << sq;

        // --- Pawn chain: υποστήριξη διαγώνια από άλλο πιόνι ---
        uint64_t supportingPawns = ((bit << 7) | (bit << 9)) & blackPawns;
        if (supportingPawns)
            blackScore += pawnChainBonus;

        // --- Backward pawn ---
        uint64_t front = bit >> 8;
        uint64_t leftSupport = bit << 7;
        uint64_t rightSupport = bit << 9;
        if (!(blackPawns & (leftSupport | rightSupport)) && !(front & ~blackPawns))
            blackScore += backwardPawnPenalty;
    }

    // --- Τελικό αποτέλεσμα ---
    return whiteScore - blackScore;
    
}


///////////// King Safety Evaluation /////////////
int king_safety_score(const BoardState& board, GamePhase phase) {
    int whiteScore = 0;
    int blackScore = 0;

    // --- Base weights ---
    int pawnShieldBonus      = 10;
    int openFilePenalty      = -15;
    int enemyProximityPenalty = -5;

    // --- Adjust for endgame ---
    if (phase == ENDGAME) {
        pawnShieldBonus /= 2;
        openFilePenalty /= 2;
        enemyProximityPenalty /= 2;
    }

    auto evaluateKing = [&](uint64_t kingBit, uint64_t ownPawns, uint64_t enemyKnights,
                            uint64_t enemyBishops, uint64_t enemyRooks, uint64_t enemyQueens,
                            bool isWhite) -> int {
        int score = 0;
        int kingSq = __builtin_ctzll(kingBit);
        int rank = kingSq / 8;
        int file = kingSq % 8;

        // --- Pawn shield ---
        uint64_t shieldMask = 0;
        if (isWhite && rank < 7) {
            if (file > 0) shieldMask |= 1ULL << (kingSq + 7);
            shieldMask |= 1ULL << (kingSq + 8);
            if (file < 7) shieldMask |= 1ULL << (kingSq + 9);
        } else if (!isWhite && rank > 0) {
            if (file > 0) shieldMask |= 1ULL << (kingSq - 9);
            shieldMask |= 1ULL << (kingSq - 8);
            if (file < 7) shieldMask |= 1ULL << (kingSq - 7);
        }
        if (ownPawns & shieldMask) score += pawnShieldBonus;

        // --- Open file penalty ---
        uint64_t fileMask = 0x0101010101010101ULL << file;
        if ((ownPawns & fileMask) == 0) score += openFilePenalty;

        // --- Nearby enemy pieces ---
        uint64_t nearbySquares = 0;
        for (int dr = -2; dr <= 2; ++dr) {
            int r = rank + dr;
            if (r < 0 || r > 7) continue;
            for (int df = -2; df <= 2; ++df) {
                int f = file + df;
                if (f < 0 || f > 7) continue;
                nearbySquares |= 1ULL << (r * 8 + f);
            }
        }
        score += __builtin_popcountll(enemyKnights & nearbySquares) * enemyProximityPenalty;
        score += __builtin_popcountll(enemyBishops & nearbySquares) * enemyProximityPenalty;
        score += __builtin_popcountll(enemyRooks   & nearbySquares) * enemyProximityPenalty;
        score += __builtin_popcountll(enemyQueens  & nearbySquares) * enemyProximityPenalty;

        return score;
    };

    whiteScore = evaluateKing(board.whiteKing, board.whitePawns,
                              board.blackKnights, board.blackBishops,
                              board.blackRooks, board.blackQueens, true);

    blackScore = evaluateKing(board.blackKing, board.blackPawns,
                              board.whiteKnights, board.whiteBishops,
                              board.whiteRooks, board.whiteQueens, false);

    return whiteScore - blackScore;
}

///////////// Main Evaluation Function /////////////
int evaluateBoard(const BoardState& board) {
    // --- 1. Υπολογισμός game phase ---
    //std::cout << "evaluating game phase..." << std::endl;
    GamePhase phase = determine_game_phase(board);

    // --- 2. Βασικό υλικό ---
    //std::cout << "evaluating material..." << std::endl;
    int material = material_score(board);

    // --- 3. Piece-Square Tables ---
    //std::cout << "evaluating piece-square tables..." << std::endl;
    int pstScore = piece_square_table_score(board, phase);

    // --- 4. Δομή πιονιών ---
    //std::cout << "evaluating pawn structure..." << std::endl;
    int pawnScore = pawn_structure_score(board, phase);

    // --- 5. Ασφάλεια βασιλιά ---
    //std::cout << "evaluating king safety..." << std::endl;
    int kingScore = king_safety_score(board, phase);

    // --- 7. Συνδυασμός όλων των scores ---
    // Μπορούμε να δώσουμε βάρη ανάλογα με τη σημασία τους
    double finalScore =
        material       * 1.5 +  // υλικό
        pstScore       * 0.8 +  // piece-square tables
        pawnScore      * 0.5 +  // δομή πιονιών
        kingScore      * 0.7   // ασφάλεια βασιλιά
    ;
    // --- 8. Προσαρμογή ανάλογα με ποιος παίζει ---
    if (!board.whiteToMove)
        finalScore = -finalScore;

    //std::cout << "final evaluation score: " << static_cast<int>(finalScore) << std::endl;
    return static_cast<int>(finalScore);
}

    