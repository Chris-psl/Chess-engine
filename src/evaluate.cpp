// This is a file that contains functions for evaluating a chess board state.

#include "evaluate.h"
#include "movegen.h"
#include <cstdint>
#include <array>
#include <iostream>

int evaluateBoard(const BoardState& board) {
    int score = 0;
    int score = material_score(board);
    if (!board.whiteToMove)
        score = -score;
    return score;



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


int piece_square_table_score(const BoardState& board) {
    // Placeholder for piece-square table evaluation
    // first we get the white pawns and check if their position is good or bad
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

    int score = 0;
    
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

    int knight_table_mid[64] = {
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

    int rook_table_mid[64] = {
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

    int bishop_table_mid[64] = {
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

  int queen_table_mid[64] = {
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

    int king_table_mid[64] = {
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

    int pawn_table_end[64] = {
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

    int knight_table_end[64] = {
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

    int rook_table_end[64] = {
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

    int bishop_table_end[64] = {
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

    int queen_table_end[64] = {
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