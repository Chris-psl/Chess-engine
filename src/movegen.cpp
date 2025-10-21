// movegen.cpp - Move generation for chess engine
// reminder: lsb = lowerest square


#include "movegen.h"
#include "parsing.h"
#include <cassert>
#include <array>
#include <iostream>


/**
 * Converts a board index (0..63) to file and rank.
 * Example: squareToCoords(0) -> (a1), 63 -> (h8)
 */
std::string squareToString(int sq) {
    char file = 'a' + (sq % 8);
    char rank = '1' + (sq / 8);
    return {file, rank};
}

// ============================================================================
//  SECTION 1: GLOBAL ATTACK TABLES
// ============================================================================
/**
 * Precomputed attack bitboards for each piece type on each square.
 * - knightAttacks[sq]: squares attacked by a knight on `sq`
 */
uint64_t knightAttacks[64];
uint64_t kingAttacks[64];
uint64_t whitePawnAttacks[64];
uint64_t blackPawnAttacks[64];

// ============================================================================
//  SECTION 2: PRECOMPUTED ATTACK MASK GENERATORS
// ============================================================================

/**
 * Returns bitboard of all squares attacked by a knight on `sq`.
 */
uint64_t maskKnightAttacks(int sq) {
    uint64_t attacks = 0ULL;
    int r = sq / 8, f = sq % 8;
    const int dr[8] = { 2, 2, 1, 1,-1,-1,-2,-2 };
    const int df[8] = { 1,-1, 2,-2, 2,-2, 1,-1 };

    for (int i = 0; i < 8; ++i) {
        int nr = r + dr[i], nf = f + df[i];
        if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8)
            SET_BIT(attacks, nr * 8 + nf);
    }
    return attacks;
}

/**
 * Returns bitboard of all squares attacked by a king on `sq`.
 */
uint64_t maskKingAttacks(int sq) {
    uint64_t attacks = 0ULL;
    int r = sq / 8, f = sq % 8;
    for (int dr = -1; dr <= 1; ++dr)
        for (int df = -1; df <= 1; ++df)
            if ((dr != 0 || df != 0) &&
                (r + dr >= 0 && r + dr < 8) &&
                (f + df >= 0 && f + df < 8))
                SET_BIT(attacks, (r + dr) * 8 + (f + df));
    return attacks;
}

/**
 * Returns bitboard of pawn attack targets for white.
 */
uint64_t maskWhitePawnAttacks(int sq) {
    uint64_t attacks = 0ULL;
    int r = sq / 8, f = sq % 8;
    if (r < 7) {
        if (f > 0) SET_BIT(attacks, sq + 7);
        if (f < 7) SET_BIT(attacks, sq + 9);
    }
    return attacks;
}

/**
 * Returns bitboard of pawn attack targets for black.
 */
uint64_t maskBlackPawnAttacks(int sq) {
    uint64_t attacks = 0ULL;
    int r = sq / 8, f = sq % 8;
    if (r > 0) {
        if (f > 0) SET_BIT(attacks, sq - 9);
        if (f < 7) SET_BIT(attacks, sq - 7);
    }
    return attacks;
}

/**
 * Initializes all attack tables. Must be called once at program startup.
 */
void initAttackTables() {
    for (int sq = 0; sq < 64; ++sq) {
        knightAttacks[sq] = maskKnightAttacks(sq);
        kingAttacks[sq]   = maskKingAttacks(sq);
        whitePawnAttacks[sq] = maskWhitePawnAttacks(sq);
        blackPawnAttacks[sq] = maskBlackPawnAttacks(sq);
    }
}

// ============================================================================
//  SECTION 3: SLIDING PIECE ATTACK GENERATION (RAYS)
// ============================================================================

/**
 * Generates ray attacks for sliding pieces (bishop, rook, queen).
 * - dr, df specify direction (e.g., bishop uses (1,1), (1,-1), etc.)
 * - blockers mask where the ray should stop.
 */
uint64_t rayAttacks(int sq, uint64_t blockers, int dr, int df) {
    uint64_t attacks = 0ULL;
    int r = sq / 8, f = sq % 8;
    while (true) {
        r += dr; f += df;
        if (r < 0 || r > 7 || f < 0 || f > 7) break; // off board
        int s = r * 8 + f;
        SET_BIT(attacks, s);
        if (GET_BIT(blockers, s)) break; // stop at first piece
    }
    return attacks;
}

/**
 * Sliding attack generators for each piece type.
 */
uint64_t bishopAttacks(int sq, uint64_t blockers) {
    return rayAttacks(sq, blockers, 1, 1) |
           rayAttacks(sq, blockers, 1, -1) |
           rayAttacks(sq, blockers, -1, 1) |
           rayAttacks(sq, blockers, -1, -1);
}

uint64_t rookAttacks(int sq, uint64_t blockers) {
    return rayAttacks(sq, blockers, 1, 0) |
           rayAttacks(sq, blockers, -1, 0) |
           rayAttacks(sq, blockers, 0, 1) |
           rayAttacks(sq, blockers, 0, -1);
}

uint64_t queenAttacks(int sq, uint64_t blockers) {
    return rookAttacks(sq, blockers) | bishopAttacks(sq, blockers);
}

// ============================================================================
//  SECTION 4: CORE MOVE GENERATION
// ============================================================================

MoveList generateMoves(const BoardState& board) {
    MoveList result;

    // ------------------------------
    // Compute useful aggregate masks
    // ------------------------------
    bool white = board.whiteToMove; // side to move

    uint64_t whitePieces = board.whitePawns | board.whiteKnights | board.whiteBishops |
                           board.whiteRooks | board.whiteQueens | board.whiteKing;

    uint64_t blackPieces = board.blackPawns | board.blackKnights | board.blackBishops |
                           board.blackRooks | board.blackQueens | board.blackKing;

    uint64_t ownPieces = white ? whitePieces : blackPieces;
    uint64_t oppPieces = white ? blackPieces : whitePieces;
    uint64_t allPieces = ownPieces | oppPieces;

    // ========================================================================
    // 5A. Compute attack masks (for both sides, for check/eval use)
    // ========================================================================
    for (int sq = 0; sq < 64; ++sq) {
        if (GET_BIT(board.whitePawns, sq)) result.whiteAttacks |= whitePawnAttacks[sq];
        if (GET_BIT(board.whiteKnights, sq)) result.whiteAttacks |= knightAttacks[sq];
        if (GET_BIT(board.whiteBishops, sq)) result.whiteAttacks |= bishopAttacks(sq, allPieces);
        if (GET_BIT(board.whiteRooks, sq))   result.whiteAttacks |= rookAttacks(sq, allPieces);
        if (GET_BIT(board.whiteQueens, sq))  result.whiteAttacks |= queenAttacks(sq, allPieces);
        if (GET_BIT(board.whiteKing, sq))    result.whiteAttacks |= kingAttacks[sq];

        if (GET_BIT(board.blackPawns, sq)) result.blackAttacks |= blackPawnAttacks[sq];
        if (GET_BIT(board.blackKnights, sq)) result.blackAttacks |= knightAttacks[sq];
        if (GET_BIT(board.blackBishops, sq)) result.blackAttacks |= bishopAttacks(sq, allPieces);
        if (GET_BIT(board.blackRooks, sq))   result.blackAttacks |= rookAttacks(sq, allPieces);
        if (GET_BIT(board.blackQueens, sq))  result.blackAttacks |= queenAttacks(sq, allPieces);
        if (GET_BIT(board.blackKing, sq))    result.blackAttacks |= kingAttacks[sq];
    }

    // ========================================================================
    // 5B. Generate legal moves for the side to move
    // ========================================================================

    /**
     * Helper to add a move to the move list.
     */
    auto addMove = [&](int from, int to, char promo, bool capture, bool ep, bool castle) {
    result.moves.push_back({from, to, promo, capture, ep, castle});
    };
    
    // ------------------------------
    // WHITE TO MOVE
    // ------------------------------
    if (white) {
        // ---- Pawn moves ----
        uint64_t pawns = board.whitePawns;
        while (pawns) {
            int from = POP_LSB(pawns);
            int rank = from / 8;

            // Forward moves
            int oneStep = from + 8;
            if (!(allPieces & (1ULL << oneStep))) {
                assert(oneStep < 64);
                if (rank == 6) {
                    for (char p : {'Q','R','B','N'})
                        addMove(from, oneStep, p, false, false, false);
                } else {
                    addMove(from, oneStep, '\0', false, false, false);
                    if (rank == 1 && !(allPieces & (1ULL << (from + 16)))){
                        addMove(from, from + 16, '\0', false, false, false);            
                    }
                }
            }

            // Captures
            uint64_t caps = whitePawnAttacks[from] & oppPieces;
            while (caps) {
                int to = POP_LSB(caps);
                if(!(board.blackKing & (1ULL << to))) { // cannot capture king
                    if (rank == 6)
                        for (char p : {'Q','R','B','N'})
                            addMove(from, to, p, true, false, false);
                    else
                        addMove(from, to, '\0', true, false, false);
                }
            }
        }

        // ---- Knights ----
        uint64_t knights = board.whiteKnights;
        printBitboard(knights);
        while (knights) {
            int from = POP_LSB(knights);
            uint64_t moves = knightAttacks[from] & ~ownPieces;
            while (moves) {
                int to = POP_LSB(moves);
                if(!(board.blackKing & (1ULL << to))){ // cannot capture king
                    addMove(from, to, '\0', GET_BIT(oppPieces, to), false, false);
                }
            }
        }

        // ---- Bishops / Rooks / Queens ----
        auto genSliding = [&](uint64_t pieces, auto attackFn) {
            uint64_t bb = pieces;
            while (bb) {
                int from = POP_LSB(bb);
                uint64_t attacks = attackFn(from, allPieces) & ~ownPieces;
                while (attacks) {
                    int to = POP_LSB(attacks);
                    if(!(board.blackKing & (1ULL << to))) { // cannot capture king
                        addMove(from, to, '\0', GET_BIT(oppPieces, to), false, false);
                    }
                }
            }
        };
        genSliding(board.whiteBishops, bishopAttacks);
        genSliding(board.whiteRooks, rookAttacks);
        genSliding(board.whiteQueens, queenAttacks);

        // ---- King ----
        int kingSq = __builtin_ctzll(board.whiteKing);
        uint64_t kingMoves = kingAttacks[kingSq] & ~ownPieces;
        while (kingMoves) {
            int to = POP_LSB(kingMoves);
            if(!(board.blackKing & (1ULL << to))){ // cannot capture king
                addMove(kingSq, to, '\0', GET_BIT(oppPieces, to), false, false);
            }
        }
    }

    // ------------------------------
    // BLACK TO MOVE
    // ------------------------------
    else {
        // ---- Pawn moves ----
        uint64_t pawns = board.blackPawns;
        while (pawns) {
            int from = POP_LSB(pawns);
            int rank = from / 8;

            // Forward moves
            int oneStep = from - 8;
            assert(oneStep >= 0);
            if (!(allPieces & (1ULL << oneStep))) {
                if (rank == 1) {
                    for (char p : {'q','r','b','n'})
                        addMove(from, oneStep, p, false, false, false);
                } else {
                    addMove(from, oneStep, '\0', false, false, false);
                    if (rank == 6 && !(allPieces & (1ULL << (from - 16))))
                        addMove(from, from - 16, '\0', false, false, false);
                }
            }

            // Captures
            uint64_t caps = blackPawnAttacks[from] & oppPieces;
            while (caps) {
                int to = POP_LSB(caps);
                if (!(board.whiteKing & 1ULL<<to)){
                    if (rank == 1)
                        for (char p : {'q','r','b','n'})
                            addMove(from, to, p, true, false, false);
                    else
                        addMove(from, to, '\0', true, false, false);
                }
            }
        }

        // ---- Knights ----
        uint64_t knights = board.blackKnights;
        while (knights ) {
            int from = POP_LSB(knights);
            uint64_t moves = knightAttacks[from] & ~ownPieces;
            while (moves) {
                int to = POP_LSB(moves);
                if(!(board.whiteKing & 1ULL<< to))
                    addMove(from, to, '\0', GET_BIT(oppPieces, to), false, false);
            }
        }

        // ---- Bishops / Rooks / Queens ----
        auto genSliding = [&](uint64_t pieces, auto attackFn) {
            uint64_t bb = pieces;
            while (bb) {
                int from = POP_LSB(bb);
                uint64_t attacks = attackFn(from, allPieces) & ~ownPieces;
                while (attacks) {
                    int to = POP_LSB(attacks);
                    if(!(board.whiteKing & 1ULL<< to))
                    addMove(from, to, '\0', GET_BIT(oppPieces, to), false, false);
                }
            }
        };
        genSliding(board.blackBishops, bishopAttacks);
        genSliding(board.blackRooks, rookAttacks);
        genSliding(board.blackQueens, queenAttacks);

        // ---- King ----
        int kingSq = __builtin_ctzll(board.blackKing);
        uint64_t kingMoves = kingAttacks[kingSq] & ~ownPieces;
        while (kingMoves) {
            int to = POP_LSB(kingMoves);
            if(!(board.whiteKing & 1ULL<< to))
            addMove(kingSq, to, '\0', GET_BIT(oppPieces, to), false, false);
        }
    }

    return result;
}
