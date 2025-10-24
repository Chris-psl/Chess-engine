// This is a file that contains functions for evaluating a chess board state.

#include "evaluate.h"
#include "movegen.h"
#include <cstdint>
#include <array>
#include <iostream>


enum GamePhase { OPENING, MIDGAME, ENDGAME };

GamePhase determine_game_phase(const BoardState& board) {
    //
    // === 1. Συντελεστές υλικού ===
    //
    constexpr int QUEEN_WEIGHT  = 9;
    constexpr int ROOK_WEIGHT   = 5;
    constexpr int BISHOP_WEIGHT = 3;
    constexpr int KNIGHT_WEIGHT = 3;
    constexpr int PAWN_WEIGHT   = 1;

    //
    // === 2. Υπολογισμός συνολικού υλικού ===
    //
    int totalMaterial = 0;
    totalMaterial += (__builtin_popcountll(board.whiteQueens) +
                      __builtin_popcountll(board.blackQueens)) * QUEEN_WEIGHT;
    totalMaterial += (__builtin_popcountll(board.whiteRooks) +
                      __builtin_popcountll(board.blackRooks)) * ROOK_WEIGHT;
    totalMaterial += (__builtin_popcountll(board.whiteBishops) +
                      __builtin_popcountll(board.blackBishops)) * BISHOP_WEIGHT;
    totalMaterial += (__builtin_popcountll(board.whiteKnights) +
                      __builtin_popcountll(board.blackKnights)) * KNIGHT_WEIGHT;
    totalMaterial += (__builtin_popcountll(board.whitePawns) +
                      __builtin_popcountll(board.blackPawns)) * PAWN_WEIGHT;

    constexpr int STARTING_MATERIAL = 39;
    double materialRatio = static_cast<double>(totalMaterial) / STARTING_MATERIAL;


    //
    // === 3. Παράγοντας γύρων ===
    //
    double moveFactor = 1.0;
    if (board.fullmoveNumber > 20) {
        moveFactor -= std::min(0.4, (board.fullmoveNumber - 20) * 0.02);
    }


    //
    // === 4. Παράγοντας δραστηριότητας και ανισορροπίας πιονιών ===
    //
    int whitePawns = __builtin_popcountll(board.whitePawns);
    int blackPawns = __builtin_popcountll(board.blackPawns);
    int pawnDiff   = std::abs(whitePawns - blackPawns);

    // Προχωρημένα πιόνια (ranks 5–7 για λευκά, 2–4 για μαύρα)
    uint64_t advancedWhite = board.whitePawns & 0x00FFFFFF00000000ULL;
    uint64_t advancedBlack = board.blackPawns & 0x000000FFFFFF0000ULL;
    int pawnActivity = __builtin_popcountll(advancedWhite) + __builtin_popcountll(advancedBlack);

    double pawnActivityFactor = 1.0 - std::min(0.3, (pawnDiff + pawnActivity) * 0.03);


    //
    // === 5. Παράγοντας κινητικότητας (mobility factor) ===
    //
    // Αν έχεις ήδη πίνακες κινητικότητας, μπορείς να τους χρησιμοποιήσεις.
    // Διαφορετικά, μια προσεγγιστική εκτίμηση:
    int estimatedMobility = 0;
    estimatedMobility += __builtin_popcountll(~board.whitePawns); // "χώρος" για λευκά
    estimatedMobility += __builtin_popcountll(~board.blackPawns); // "χώρος" για μαύρα
    double mobilityFactor = std::min(1.0, static_cast<double>(estimatedMobility) / 800.0);


    //
    // === 6. Παράγοντας ανοιχτών στηλών ===
    //
    uint64_t allPawns = board.whitePawns | board.blackPawns;
    int openFiles = 0;
    for (int file = 0; file < 8; ++file) {
        uint64_t fileMask = 0x0101010101010101ULL << file;
        if ((allPawns & fileMask) == 0)
            openFiles++;
    }
    double openFileFactor = static_cast<double>(openFiles) / 8.0;


    //
    // === 7. Παρουσία βασιλισσών ===
    //
    bool queensExist = (__builtin_popcountll(board.whiteQueens | board.blackQueens) > 0);
    double queenFactor = queensExist ? 1.0 : 0.5;


    //
    // === 8. Συνδυασμός όλων των παραγόντων ===
    //
    double phaseScore =
        (materialRatio      * 0.50) +   // υλικό
        (moveFactor         * 0.20) +   // γύροι
        (pawnActivityFactor * 0.10) +   // δραστηριότητα πιονιών
        (mobilityFactor     * 0.10) +   // κινητικότητα
        (openFileFactor     * 0.05) +   // ανοιχτές στήλες
        (queenFactor        * 0.05);    // ύπαρξη βασιλισσών


    //
    // === 9. Τελική κατηγοριοποίηση ===
    //
    if (phaseScore > 0.7)
        return OPENING;
    else if (phaseScore > 0.4)
        return MIDGAME;
    else
        return ENDGAME;
}


int evaluateBoard(const BoardState& board) {
    // --- 1. Υπολογισμός game phase ---
    GamePhase phase = determine_game_phase(board);

    // --- 2. Βασικό υλικό ---
    int material = material_score(board);

    // --- 3. Piece-Square Tables ---
    int pstScore = piece_square_table_score(board, phase);

    // --- 4. Δομή πιονιών ---
    int pawnScore = pawn_structure_score(board, phase);

    // --- 5. Ασφάλεια βασιλιά ---
    int kingScore = king_safety_score(board, phase);

    // --- 6. Κινητικότητα ---
    int mobilityScore = mobility_score(board, phase); // υποθέτουμε ότι η συνάρτηση υλοποιείται on pending 

    // --- 7. Συνδυασμός όλων των scores ---
    // Μπορούμε να δώσουμε βάρη ανάλογα με τη σημασία τους
    double finalScore =
        material       * 1.0 +  // υλικό
        pstScore       * 0.8 +  // piece-square tables
        pawnScore      * 0.5 +  // δομή πιονιών
        kingScore      * 0.7 +  // ασφάλεια βασιλιά
        mobilityScore  * 0.3;   // κινητικότητα

    // --- 8. Προσαρμογή ανάλογα με ποιος παίζει ---
    if (!board.whiteToMove)
        finalScore = -finalScore;

    return static_cast<int>(finalScore);
}




int material_score(const BoardState& board) {
    const int PAWN_VALUE   = 100;
    const int KNIGHT_VALUE = 320;
    const int BISHOP_VALUE = 330;
    const int ROOK_VALUE   = 500;
    const int QUEEN_VALUE  = 900;

    int whiteScore = 0;
    whiteScore += __builtin_popcountll(board.whitePawns)   * PAWN_VALUE;
    whiteScore += __builtin_popcountll(board.whiteKnights) * KNIGHT_VALUE;
    whiteScore += __builtin_popcountll(board.whiteBishops) * BISHOP_VALUE;
    whiteScore += __builtin_popcountll(board.whiteRooks)   * ROOK_VALUE;
    whiteScore += __builtin_popcountll(board.whiteQueens)  * QUEEN_VALUE;

    int blackScore = 0;
    blackScore += __builtin_popcountll(board.blackPawns)   * PAWN_VALUE;
    blackScore += __builtin_popcountll(board.blackKnights) * KNIGHT_VALUE;
    blackScore += __builtin_popcountll(board.blackBishops) * BISHOP_VALUE;
    blackScore += __builtin_popcountll(board.blackRooks)   * ROOK_VALUE;
    blackScore += __builtin_popcountll(board.blackQueens)  * QUEEN_VALUE;

    return whiteScore - blackScore;
    
}


int piece_square_table_score(const BoardState& board, GamePhase phase) {
    // Placeholder for piece-square table evaluation
    // first we get the white pawns and check if their position is good or bad
    if (phase == OPENING){
    int score = 0;
        static const int pawnTable_opening[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 27, 27, 10,  5,  5,
     0,  0,  0, 25, 25,  0,  0,  0,
     5, -5,-10,  0 ,  0,-10, -5,  5,
     5, 10, 10,-25,-25, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};
    uint64_t white_pawns = board.whitePawns;
    while (white_pawns) {
        int sq = POP_LSB(white_pawns);
        score += pawnTable_opening[sq]; // λευκό


    }
    uint64_t black_pawns = board.blackPawns;
    while (black_pawns) {
        int sq = POP_LSB(black_pawns);
        score -= pawnTable_opening[63 - sq]; // μαύρο
    
    }

    static const int knight_table_opening[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-20,-30,-30,-20,-40,-50,    
};
    uint64_t white_knights = board.whiteKnights;
    while (white_knights) {
        int sq = POP_LSB(white_knights);
        score += knight_table_opening[sq]; // λευκό
    }
    uint64_t black_knights = board.blackKnights;
    while (black_knights) {
        int sq = POP_LSB(black_knights);
        score -= knight_table_opening[63 - sq]; // μαύρο

    }

    static const int rook_table_opening[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       10, 10, 10, 10, 10, 10, 10, 10,
        0,  0,  0,  5,  5,  0,  0,  0
    };
    uint64_t white_rooks = board.whiteRooks;
    while (white_rooks) {
        int sq = POP_LSB(white_rooks);
        score += rook_table_opening[sq]; // λευκό
    }
    uint64_t black_rooks = board.blackRooks;
    while (black_rooks) {
        int sq = POP_LSB(black_rooks);
        score -= rook_table_opening[63 - sq]; // μαύρο
    }

    const int bishop_table_opening[64] = {
       -20,-10,-10,-10,-10,-10,-10,-20,
       -10,  0,  0,  0,  0,  0,  0,-10,
       -10,  0,  5, 10, 10,  5,  0,-10,
       -10,  5,  5, 10, 10,  5,  5,-10,
       -10,  0, 10, 10, 10, 10,  0,-10,
       -10, 10, 10, 10, 10, 10, 10,-10,
       -10,  5,  0,  0,  0,  0,  5,-10,
       -20,-10,-40,-10,-10,-40,-10,-20,
    };

    uint64_t white_bishops = board.whiteBishops;

    while (white_bishops) {
        int sq = POP_LSB(white_bishops);
        score += bishop_table_opening[sq]; // λευκό
    }

    uint64_t black_bishops = board.blackBishops;

    while (black_bishops) {
        int sq = POP_LSB(black_bishops);
        score -= bishop_table_opening[63 - sq]; // μαύρο
    }

    static const int queen_table_opening[64] = {
       -5, -5, -5, -2, -2, -5, -5, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  5,  5,  5,  5,  0, -5,
       -2,  0,  5, 10, 10,  5,  0, -2,
       -2,  0,  5, 10, 10,  5,  0, -2,
       -5,  0,  5,  5,  5,  5,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5, -5, -5, -2, -2, -5, -5, -5
    };

    uint64_t white_queens = board.whiteQueens;

    while (white_queens) {
        int sq = POP_LSB(white_queens);
        score += queen_table_opening[sq]; // λευκό
    }

    uint64_t black_queens = board.blackQueens;

    while (black_queens) {
        int sq = POP_LSB(black_queens);
        score -= queen_table_opening[63 - sq]; // μαύρο
    }

    static const int king_table_opening[64] = {
       -30,-40,-40,-50,-50,-40,-40,-30,
       -30,-40,-40,-50,-50,-40,-40,-30,
       -30,-40,-40,-50,-50,-40,-40,-30,            
       -30,-40,-40,-50,-50,-40,-40,-30,
       -20,-30,-30,-40,-40,-30,-30,-20,
       -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20,  0,  0,  0,  0, 20, 20,
        20, 30, 10,  0,  0, 10, 30, 20     
    };

    uint64_t white_king = board.whiteKing;

    while (white_king) {
        int sq = POP_LSB(white_king);
        score += king_table_opening[sq]; // λευκό
    }
    uint64_t black_king = board.blackKing;
    while (black_king) {
        int sq = POP_LSB(black_king);
        score -= king_table_opening[63 - sq]; // μαύρο
    }
    return score;

}

    int score = 0;
    if (phase == MIDGAME) {
    static const int pawnTable_mid[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 27, 27, 10,  5,  5,
     0,  0,  0, 25, 25,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-25,-25, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};

    
    
    uint64_t white_pawns = board.whitePawns;
    while (white_pawns) {
        int sq = POP_LSB(white_pawns);
        score += pawnTable_mid[sq]; // λευκό
    }
    uint64_t black_pawns = board.blackPawns;
    while (black_pawns) {
        int sq = POP_LSB(black_pawns);
        score -= pawnTable_mid[63 - sq]; // μαύρο
    }

    static const int knight_table_mid[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-20,-30,-30,-20,-40,-50,
};

    uint64_t white_knights = board.whiteKnights;
    while (white_knights) {
        int sq = POP_LSB(white_knights);
        score += knight_table_mid[sq]; // λευκό
    }
    uint64_t black_knights = board.blackKnights;
    while (black_knights) {
        int sq = POP_LSB(black_knights);
        score -= knight_table_mid[63 - sq]; // μαύρο
    }

    static const int rook_table_mid[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    10, 10, 10, 10, 10, 10, 10, 10,
     0,  0,  0,  5,  5,  0,  0,  0
};

    uint64_t white_rooks = board.whiteRooks;
    while (white_rooks) {
        int sq = POP_LSB(white_rooks);
        score += rook_table_mid[sq]; // λευκό
    }
    uint64_t black_rooks = board.blackRooks;
    while (black_rooks) {
        int sq = POP_LSB(black_rooks);
        score -= rook_table_mid[63 - sq]; // μαύρο
    }

    static const int bishop_table_mid[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-40,-10,-10,-40,-10,-20,
};

    uint64_t white_bishops = board.whiteBishops;
    while (white_bishops) {
        int sq = POP_LSB(white_bishops);
        score += bishop_table_mid[sq]; // λευκό
    }
    uint64_t black_bishops = board.blackBishops;
    while (black_bishops) {
        int sq = POP_LSB(black_bishops);
        score -= bishop_table_mid[63 - sq]; // μαύρο
    }

  static const int queen_table_mid[64] = {
    -5, -5, -5, -2, -2, -5, -5, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  5,  5,  5,  5,  0, -5,
    -2,  0,  5, 10, 10,  5,  0, -2,
    -2,  0,  5, 10, 10,  5,  0, -2,
    -5,  0,  5,  5,  5,  5,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5, -5, -5, -2, -2, -5, -5, -5
};

    uint64_t white_queens = board.whiteQueens;
    while (white_queens) {
        int sq = POP_LSB(white_queens);
        score += queen_table_mid[sq]; // λευκό
    }
    uint64_t black_queens = board.blackQueens;
    while (black_queens) {
        int sq = POP_LSB(black_queens);
        score -= queen_table_mid[63 - sq]; // μαύρο
    }

    static const int king_table_mid[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,            
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20     
};
    uint64_t white_king = board.whiteKing;
    while (white_king) {
        int sq = POP_LSB(white_king);
        score += king_table_mid[sq]; // λευκό
    }
    uint64_t black_king = board.blackKing;
    while (black_king) {
        int sq = POP_LSB(black_king);
        score -= king_table_mid[63 - sq]; // μαύρο
    }
    return score;

}
  else if (phase == ENDGAME) {
    int score = 0;
    static const int pawn_table_end[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
       50, 50, 50, 50, 50, 50, 50, 50,
       10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 27, 27, 10,  5,  5,          
        0,  0,  0, 25, 25,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
       10, 10, 10,-25,-25, 10, 10, 10,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    uint64_t white_pawns_end = board.whitePawns;
    while (white_pawns_end) {
        int sq = POP_LSB(white_pawns_end);
        score += pawn_table_end[sq]; // λευκό
    }   
    uint64_t black_pawns_end = board.blackPawns;
    while (black_pawns_end) {
        int sq = POP_LSB(black_pawns_end);
        score -= pawn_table_end[63 - sq]; // μαύρο
    }

    int king_table_end[64] = {
         -50,-40,-30,-20,-20,-30,-40,-50,
         -30,-20,-10,  0,  0,-10,-20,-30,
         -30,-10, 20, 30, 30, 20,-10,-30,
         -30,-10, 30, 40, 40, 30,-10,-30,
         -30,-10, 30, 40, 40, 30,-10,-30,
         -30,-10, 20, 30, 30, 20,-10,-30,
         -30,-30,  0,  0,  0,  0,-30,-30,
         -50,-40,-30,-20,-20,-30,-40,-50
     };
    uint64_t white_king_end = board.whiteKing;

    while (white_king_end) {
        int sq = POP_LSB(white_king_end);
        score += king_table_end[sq]; // λευκό
    }
    uint64_t black_king_end = board.blackKing;

    while (black_king_end) {
        int sq = POP_LSB(black_king_end);
        score -= king_table_end[63 - sq]; // μαύρο
    }

    static const int knight_table_end[64] = {
     -50,-40,-30,-30,-30,-30,-40,-50,
     -40,-20,  0,  0,  0,  0,-20,-40,
     -30,  0, 10, 15, 15, 10,  0,-30,
     -30,  5, 15, 20, 20, 15,  5,-30,
     -30,  0, 15, 20, 20, 15,  0,-30,
     -30,  5, 10, 15, 15, 10,  5,-30,
     -40,-20,  0,  5,  5,  0,-20,-40,
     -50,-40,-20,-30,-30,-20,-40,-50,
    };

    uint64_t white_knights_end = board.whiteKnights;


    while (white_knights_end) {
        int sq = POP_LSB(white_knights_end);
        score += knight_table_end[sq]; // λευκό
    }

    uint64_t black_knights_end = board.blackKnights;
    while (black_knights_end) {
        int sq = POP_LSB(black_knights_end);
        score -= knight_table_end[63 - sq]; // μαύρο

    }

    static const int rook_table_end[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       10, 10, 10, 10, 10, 10, 10, 10,
        0,  0,  0,  5,  5,  0,  0,  0
    };

    uint64_t white_rooks_end = board.whiteRooks;

    while (white_rooks_end) {
        int sq = POP_LSB(white_rooks_end);
        score += rook_table_end[sq]; // λευκό
    }

    uint64_t black_rooks_end = board.blackRooks;

    while (black_rooks_end) {
        int sq = POP_LSB(black_rooks_end);
        score -= rook_table_end[63 - sq]; // μαύρο
    }

    static const int bishop_table_end[64] = {
       -20,-10,-10,-10,-10,-10,-10,-20,
       -10,  0,  0,  0,  0,  0,  0,-10,
       -10,  0,  5, 10, 10,  5,  0,-10,
       -10,  5,  5, 10, 10,  5,  5,-10,
       -10,  0, 10, 10, 10, 10,  0,-10,
       -10, 10, 10, 10, 10, 10, 10,-10,
       -10,  5,  0,  0,  0,  0,  5,-10,
       -20,-10,-40,-10,-10,-40,-10,-20,
    };

    uint64_t white_bishops_end = board.whiteBishops;
    while (white_bishops_end) {
        int sq = POP_LSB(white_bishops_end);
        score += bishop_table_end[sq]; // λευκό
    }

    uint64_t black_bishops_end = board.blackBishops;
    while (black_bishops_end) {
        int sq = POP_LSB(black_bishops_end);
        score -= bishop_table_end[63 - sq]; // μαύρο
    }

    static const int queen_table_end[64] = {
       -5, -5, -5, -2, -2, -5, -5, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  5,  5,  5,  5,  0, -5,
       -2,  0,  5, 10, 10,  5,  0, -2,
       -2,  0,  5, 10, 10,  5,  0, -2,
       -5,  0,  5,  5,  5,  5,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5, -5, -5, -2, -2, -5, -5, -5
    };

    uint64_t white_queens_end = board.whiteQueens;
    while (white_queens_end) {
        int sq = POP_LSB(white_queens_end);
        score += queen_table_end[sq]; // λευκό
    }   

    uint64_t black_queens_end = board.blackQueens;
    while (black_queens_end) {
        int sq = POP_LSB(black_queens_end);
        score -= queen_table_end[63 - sq]; // μαύρο
    }

    return score;

  }
}


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



int king_safety_score(const BoardState& board, GamePhase phase) {
    int whiteScore = 0;
    int blackScore = 0;

    // --- Σταθερές weights ---
    int pawnShieldBonus   = 10;   // Bonus αν υπάρχει προστατευτικό πιονιών μπροστά από τον βασιλιά
    int openFilePenalty   = -15;  // Ποινή αν υπάρχει ανοιχτή στήλη κοντά στον βασιλιά
    int enemyProximityPenalty = -5; // Ποινή για κάθε εχθρικό κομμάτι κοντά στον βασιλιά

    // --- Προσαρμογή ανά game phase ---
    if (phase == ENDGAME) {
        // Στο endgame οι βασιλιάδες είναι πιο ενεργοί → μειώνουμε την ποινή
        pawnShieldBonus /= 2;
        openFilePenalty /= 2;
        enemyProximityPenalty /= 2;
    }

    // --- Βασιλιάς λευκών ---
    uint64_t whiteKingBit = board.whiteKing;
    int kingSq = __builtin_ctzll(whiteKingBit);

    // Υπολογισμός προστατευτικού πιονιών μπροστά
    uint64_t shieldMask = 0;
    if (kingSq / 8 > 0) {
        if ((kingSq % 8) > 0)
            shieldMask |= 1ULL << (kingSq - 9); // μπροστά αριστερά
        shieldMask |= 1ULL << (kingSq - 8);     // μπροστά
        if ((kingSq % 8) < 7)
            shieldMask |= 1ULL << (kingSq - 7); // μπροστά δεξιά
    }
    if (board.whitePawns & shieldMask)
        whiteScore += pawnShieldBonus;

    // Ποινή αν υπάρχει ανοιχτή στήλη
    int file = kingSq % 8;
    uint64_t fileMask = 0x0101010101010101ULL << file;
    if ((board.whitePawns & fileMask) == 0)
        whiteScore += openFilePenalty;

    // Ποινή για κοντινά εχθρικά κομμάτια (1-2 τετράγωνα γύρω)
    uint64_t nearbySquares = 0;
    int rank = kingSq / 8;
    for (int dr = -2; dr <= 2; ++dr) {
        int r = rank + dr;
        if (r < 0 || r > 7) continue;
        for (int df = -2; df <= 2; ++df) {
            int f = file + df;
            if (f < 0 || f > 7) continue;
            nearbySquares |= 1ULL << (r * 8 + f);
        }
    }
    whiteScore += __builtin_popcountll(board.blackKnights & nearbySquares) * enemyProximityPenalty;
    whiteScore += __builtin_popcountll(board.blackBishops & nearbySquares) * enemyProximityPenalty;
    whiteScore += __builtin_popcountll(board.blackRooks   & nearbySquares) * enemyProximityPenalty;
    whiteScore += __builtin_popcountll(board.blackQueens  & nearbySquares) * enemyProximityPenalty;

    // --- Βασιλιάς μαύρων (αντίστοιχα) ---
    uint64_t blackKingBit = board.blackKing;
    kingSq = __builtin_ctzll(blackKingBit);
    shieldMask = 0;
    if (kingSq / 8 < 7) {
        if ((kingSq % 8) > 0)
            shieldMask |= 1ULL << (kingSq + 7); // μπροστά αριστερά
        shieldMask |= 1ULL << (kingSq + 8);     // μπροστά
        if ((kingSq % 8) < 7)
            shieldMask |= 1ULL << (kingSq + 9); // μπροστά δεξιά
    }
    if (board.blackPawns & shieldMask)
        blackScore += pawnShieldBonus;

    file = kingSq % 8;
    fileMask = 0x0101010101010101ULL << file;
    if ((board.blackPawns & fileMask) == 0)
        blackScore += openFilePenalty;

    rank = kingSq / 8;
    nearbySquares = 0;
    for (int dr = -2; dr <= 2; ++dr) {
        int r = rank + dr;
        if (r < 0 || r > 7) continue;
        for (int df = -2; df <= 2; ++df) {
            int f = file + df;
            if (f < 0 || f > 7) continue;
            nearbySquares |= 1ULL << (r * 8 + f);
        }
    }
    blackScore += __builtin_popcountll(board.whiteKnights & nearbySquares) * enemyProximityPenalty;
    blackScore += __builtin_popcountll(board.whiteBishops & nearbySquares) * enemyProximityPenalty;
    blackScore += __builtin_popcountll(board.whiteRooks   & nearbySquares) * enemyProximityPenalty;
    blackScore += __builtin_popcountll(board.whiteQueens  & nearbySquares) * enemyProximityPenalty;

    return whiteScore - blackScore;
}


// Εκτίμηση κινητικότητας κομματιών με βάρη ανά τύπο κομματιού
int mobility_score(const BoardState& board, GamePhase phase) {
    int mobility = 0;

    uint64_t allPieces = board.whitePawns | board.whiteKnights | board.whiteBishops |
                         board.whiteRooks | board.whiteQueens | board.whiteKing |
                         board.blackPawns | board.blackKnights | board.blackBishops |
                         board.blackRooks | board.blackQueens | board.blackKing;

    // --- Βάρη για διαφορετικά κομμάτια ---
    constexpr int KNIGHT_WEIGHT = 1;
    constexpr int BISHOP_WEIGHT = 1;
    constexpr int ROOK_WEIGHT   = 2;
    constexpr int QUEEN_WEIGHT  = 3;

    // --- Λευκά ---
    uint64_t temp;

    // Ιππότες
    temp = board.whiteKnights;
    while (temp) {
        int sq = POP_LSB(temp);
        // Προσεγγιστικά: ιππότης μπορεί να κινηθεί σε άδεια τετράγωνα γύρω του
        mobility += __builtin_popcountll(~allPieces) * KNIGHT_WEIGHT;
    }

    // Αξιωματικοί
    temp = board.whiteBishops;
    while (temp) {
        int sq = POP_LSB(temp);
        mobility += __builtin_popcountll(~allPieces) * BISHOP_WEIGHT;
    }

    // Πύργοι
    temp = board.whiteRooks;
    while (temp) {
        int sq = POP_LSB(temp);
        mobility += __builtin_popcountll(~allPieces) * ROOK_WEIGHT;
    }

    // Βασίλισσες
    temp = board.whiteQueens;
    while (temp) {
        int sq = POP_LSB(temp);
        mobility += __builtin_popcountll(~allPieces) * QUEEN_WEIGHT;
    }

    // --- Μαύρα ---
    temp = board.blackKnights;
    while (temp) {
        int sq = POP_LSB(temp);
        mobility += __builtin_popcountll(~allPieces) * KNIGHT_WEIGHT;
    }

    temp = board.blackBishops;
    while (temp) {
        int sq = POP_LSB(temp);
        mobility += __builtin_popcountll(~allPieces) * BISHOP_WEIGHT;
    }

    temp = board.blackRooks;
    while (temp) {
        int sq = POP_LSB(temp);
        mobility += __builtin_popcountll(~allPieces) * ROOK_WEIGHT;
    }

    temp = board.blackQueens;
    while (temp) {
        int sq = POP_LSB(temp);
        mobility += __builtin_popcountll(~allPieces) * QUEEN_WEIGHT;
    }

    // --- Normalization ---
    // Κάνουμε το mobility να έχει εύρος ~0–100
    double mobilityFactor = std::min(1.0, static_cast<double>(mobility) / 1500.0);

    return static_cast<int>(mobilityFactor * 100);
}


    