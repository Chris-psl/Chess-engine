// movegen.cpp - Move generation for chess engine
// reminder: lsb = lowerest square


#include "movegen.h"
#include "parsing.h"
#include "updateBoard.h"

#include <bitset>
#include <cassert>
#include <array>
#include <iostream>
#include <algorithm>

/**
 * Converts a board index (0..63) to file and rank.
 * Example: squareToCoords(0) -> (a1), 63 -> (h8)
 */
std::string squareToString(int sq) {
    char file = 'a' + (sq % 8); // mod
    char rank = '1' + (sq / 8); // div
    return {file, rank};
}

/**
 * Macros for bitboard manipulation
 * - GET_BIT(bb, sq): returns 1 if bit at `sq` is set in `bb`
 */
static inline uint64_t BIT(int sq) {
    return (sq >= 0 && sq < 64) ? (1ULL << sq) : 0ULL;
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

/*
 * Generates all attacks for the given board state.
 */
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
    std::vector<Move> captures;
    std::vector<Move> quiets;
    auto addMove = [&](int from, int to, char promo, bool capture, bool ep, bool castle) {
        Move m = {from, to, promo, capture, ep, castle};
        if (capture || ep || promo != '\0')
            captures.push_back(m);
        else if(!board.genVolatile)
            quiets.push_back(m);
    };
    
    // ------------------------------
    // WHITE TO MOVE
    // ------------------------------
    if (white) {
        // En passant captures
        if (board.enPassantSquare != -1) {
            int ep = board.enPassantSquare;
            int from1 = ep - 9, from2 = ep - 7;
            if (from1 >= 0 && from1 < 64 && GET_BIT(board.whitePawns, from1))
                addMove(from1, ep, '\0', true, true, false);
            if (from2 >= 0 && from2 < 64 && GET_BIT(board.whitePawns, from2))
                addMove(from2, ep, '\0', true, true, false);
        }

        // ---- Pawn moves ----
        uint64_t pawns = board.whitePawns;
        while (pawns) {
            int from = POP_LSB(pawns);
            int rank = from / 8;

            // Captures
            uint64_t caps = whitePawnAttacks[from] & oppPieces;
            while (caps) {
                int to = POP_LSB(caps);
                if(!(board.blackKing & BIT(to))) { // cannot capture king
                    if (rank == 6)
                        for (char p : {'Q','R','B','N'})
                            addMove(from, to, p, true, false, false);
                    else
                        addMove(from, to, '\0', true, false, false);
                }
            }

            // Forward moves
            int oneStep = from + 8;
            if (!(allPieces & BIT(oneStep))) {
                assert(oneStep < 64);
                if (rank == 6) {
                    for (char p : {'Q','R','B','N'})
                        addMove(from, oneStep, p, false, false, false);
                } else {
                    addMove(from, oneStep, '\0', false, false, false);
                    if (rank == 1 && !(allPieces & BIT(from + 16))) {
                        addMove(from, from + 16, '\0', false, false, false);            
                    }
                }
            }
        }

        // Function to add castling moves
        if(board.castlingRights != "no_castling") {
            int kingFrom = __builtin_ctzll(board.whiteKing);
            // Kingside castling
            if (board.castlingRights.find('K') != std::string::npos) {
                if (!GET_BIT(allPieces, 5) && !GET_BIT(allPieces, 6)) {
                    // Check squares are not under attack
                    if (!(result.blackAttacks & BIT(4)) &&
                        !(result.blackAttacks & BIT(5)) &&
                        !(result.blackAttacks & BIT(6))) {
                        addMove(kingFrom, 6, '\0', false, false, true);
                    }
                }
            }
            // Queenside castling
            if (board.castlingRights.find('Q') != std::string::npos) {
                if (!GET_BIT(allPieces, 1) && !GET_BIT(allPieces, 2) && !GET_BIT(allPieces, 3)) {
                    // Check squares are not under attack
                    if (!(result.blackAttacks & BIT(4)) &&
                        !(result.blackAttacks & BIT(3)) &&
                        !(result.blackAttacks & BIT(2))) {
                        addMove(kingFrom, 2, '\0', false, false, true);
                    }
                }
            }
        }


        // ---- Knights ----
        uint64_t knights = board.whiteKnights;
        while (knights) {
            int from = POP_LSB(knights);
            uint64_t moves = knightAttacks[from] & ~ownPieces;
            while (moves) {
                int to = POP_LSB(moves);
                if(!(board.blackKing & BIT(to))){ // cannot capture king
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
                    if(!(board.blackKing & BIT(to))) { // cannot capture king
                        addMove(from, to, '\0', GET_BIT(oppPieces, to), false, false);
                    }
                }
            }
        };
        genSliding(board.whiteBishops, bishopAttacks);
        genSliding(board.whiteRooks, rookAttacks);
        genSliding(board.whiteQueens, queenAttacks);

        // ---- King ----
        if(board.whiteKing != 0){
            int kingSq = __builtin_ctzll(board.whiteKing);
            uint64_t kingMoves = kingAttacks[kingSq] & ~ownPieces;
            while (kingMoves) {
                int to = POP_LSB(kingMoves);
                if(!(board.blackKing & BIT(to))){ // cannot capture king
                    addMove(kingSq, to, '\0', GET_BIT(oppPieces, to), false, false);
                }
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
            if (!(allPieces & BIT(oneStep))) {
                if (rank == 1) {
                    for (char p : {'q','r','b','n'})
                        addMove(from, oneStep, p, false, false, false);
                } else {
                    addMove(from, oneStep, '\0', false, false, false);
                    if (rank == 6 && !(allPieces & BIT(from - 16)))
                        addMove(from, from - 16, '\0', false, false, false);
                }
            }

            // Captures
            uint64_t caps = blackPawnAttacks[from] & oppPieces;
            while (caps) {
                int to = POP_LSB(caps);
                if (!(board.whiteKing & BIT(to))){
                    if (rank == 1)
                        for (char p : {'q','r','b','n'})
                            addMove(from, to, p, true, false, false);
                    else
                        addMove(from, to, '\0', true, false, false);
                }
            }
        }

        // En passant captures
        if (board.enPassantSquare != -1) {
            int from1 = board.enPassantSquare + 9;
            int from2 = board.enPassantSquare + 7;
            
            if(GET_BIT(board.blackPawns, from1)) {
                addMove(from1, board.enPassantSquare, '\0', true, true, false);
            }
            if(GET_BIT(board.blackPawns, from2)) {
                addMove(from2, board.enPassantSquare, '\0', true, true, false);
            }
        }

        // Function to add castling moves
        if(board.castlingRights != "no_castling") {
            int kingFrom = __builtin_ctzll(board.blackKing);
            // Kingside castling
            if (board.castlingRights.find('k') != std::string::npos) {
                if (!GET_BIT(allPieces, 61) && !GET_BIT(allPieces, 62)) {
                    // Check squares are not under attack
                    if (!(result.whiteAttacks & BIT(60)) &&
                        !(result.whiteAttacks & BIT(61)) &&
                        !(result.whiteAttacks & BIT(62))) {
                        addMove(kingFrom, 62, '\0', false, false, true);
                    }
                }
            }
            // Queenside castling
            if (board.castlingRights.find('q') != std::string::npos) {
                if (!GET_BIT(allPieces, 57) && !GET_BIT(allPieces, 58) && !GET_BIT(allPieces, 59)) {
                    // Check squares are not under attack
                    if (!(result.whiteAttacks & BIT(60)) &&
                        !(result.whiteAttacks & BIT(59)) &&
                        !(result.whiteAttacks & BIT(58))) {
                        addMove(kingFrom, 58, '\0', false, false, true);
                    }
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
                if(!(board.whiteKing & BIT(to)))
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
                    if(!(board.whiteKing & BIT(to)))
                    addMove(from, to, '\0', GET_BIT(oppPieces, to), false, false);
                }
            }
        };
        genSliding(board.blackBishops, bishopAttacks);
        genSliding(board.blackRooks, rookAttacks);
        genSliding(board.blackQueens, queenAttacks);

        // ---- King ----
        if(board.blackKing != 0){
            int kingSq = __builtin_ctzll(board.blackKing);
            uint64_t kingMoves = kingAttacks[kingSq] & ~ownPieces;
            while (kingMoves) {
                int to = POP_LSB(kingMoves);
                if(!(board.whiteKing & BIT(to)))
                addMove(kingSq, to, '\0', GET_BIT(oppPieces, to), false, false);
            }
        }
    }

    // Combine captures and quiet moves, captures first
    result.moves.reserve(captures.size() + quiets.size());
    result.moves.insert(result.moves.end(), captures.begin(), captures.end());
    result.moves.insert(result.moves.end(), quiets.begin(), quiets.end());

    return result;
}

/**
 * Function that ensures the king is not exposed to check after move generation.
 */
bool isLegalMoveState(const BoardState& board) {
     // Both kings must exist first (avoid __builtin_ctzll on zero)
    if (board.whiteKing == 0 || board.blackKing == 0) return false;

    // We need to check the side that just moved (opposite of side to move)
    bool white = !board.whiteToMove;

    // Get that side's king square
    int kingSq = white ? __builtin_ctzll(board.whiteKing) : __builtin_ctzll(board.blackKing);

    // Collect all pieces
    uint64_t allPieces = (board.whitePawns | board.whiteKnights | board.whiteBishops |
                          board.whiteRooks | board.whiteQueens | board.whiteKing |
                          board.blackPawns | board.blackKnights | board.blackBishops |
                          board.blackRooks | board.blackQueens | board.blackKing);

    // Collect opponent pieces (the side that will move next)
    uint64_t oppPawns   = white ? board.blackPawns   : board.whitePawns;
    uint64_t oppKnights = white ? board.blackKnights : board.whiteKnights;
    uint64_t oppBishops = white ? board.blackBishops : board.whiteBishops;
    uint64_t oppRooks   = white ? board.blackRooks   : board.whiteRooks;
    uint64_t oppQueens  = white ? board.blackQueens  : board.whiteQueens;
    uint64_t oppKing    = white ? board.blackKing    : board.whiteKing;

    // Check for attacks on the king
    // kingSq is 0..63
    auto pawnAttacksTo = [&](int attackerIsWhite, int targetSq)->uint64_t {
        // return bitboard of pawn squares (attacker color) that attack targetSq
        uint64_t res = 0ULL;
        int r = targetSq / 8, f = targetSq % 8;
        if (attackerIsWhite) {
            // white pawns attack up-left and up-right (from lower rank to higher)
            if (r - 1 >= 0) {
                if (f - 1 >= 0) res |= BIT((r - 1) * 8 + (f - 1));
                if (f + 1 <= 7) res |= BIT((r - 1) * 8 + (f + 1));
            }
        } else {
            // black pawns attack down-left and down-right (from higher rank to lower)
            if (r + 1 <= 7) {
                if (f - 1 >= 0) res |= BIT((r + 1) * 8 + (f - 1));
                if (f + 1 <= 7) res |= BIT((r + 1) * 8 + (f + 1));
            }
        }
        return res;
    };

    // In isLegalMoveState:
    if (white) {
        // side that just moved is white -> check if any black pawn attacks white king
        if (pawnAttacksTo(/*attackerIsWhite=*/false, kingSq) & oppPawns) return false;
    } else {
        if (pawnAttacksTo(/*attackerIsWhite=*/true, kingSq) & oppPawns) return false;
    }

    if (knightAttacks[kingSq] & oppKnights) return false;
    if (bishopAttacks(kingSq, allPieces) & (oppBishops | oppQueens)) return false;
    if (rookAttacks(kingSq, allPieces) & (oppRooks | oppQueens)) return false;
    if (kingAttacks[kingSq] & oppKing) return false;

    return true;
}

/**
 * Move ordering function that prioritizes captures then promotions then quiet moves.
 * It also sorts captures by MVV-LVA (Most Valuable Victim - Least Valuable Attacker).
 */
MoveList orderMoves(const MoveList& moves, const BoardState& board) {
    MoveList ordered;
    ordered.moves = moves.moves; // copy moves

    // Simple scoring function for MVV-LVA
    auto scoreMove = [&](const Move& move) -> int {
        int score = 0;
        if (move.isCapture) {
            // Assign values to pieces for scoring
            auto pieceValue = [&](int sq) -> int {
                if (GET_BIT(board.whitePawns | board.blackPawns, sq)) return 100;
                if (GET_BIT(board.whiteKnights | board.blackKnights, sq)) return 320;
                if (GET_BIT(board.whiteBishops | board.blackBishops, sq)) return 330;
                if (GET_BIT(board.whiteRooks | board.blackRooks, sq)) return 500;
                if (GET_BIT(board.whiteQueens | board.blackQueens, sq)) return 900;
                return 0; // king or empty
            };
            int victimValue = pieceValue(move.to);
            int attackerValue = pieceValue(move.from);
            score += (victimValue * 10) - attackerValue; // MVV-LVA
        }
        if (move.promotion != '\0') {
            score += 800; // high score for promotions
        }
        return score;
    };

    // Sort moves based on their scores
    std::sort(ordered.moves.begin(), ordered.moves.end(),
              [&](const Move& a, const Move& b) {
                  return scoreMove(a) > scoreMove(b);
              });

    return ordered;
}



/**
 * Generate all legal moves for the current player.
 */
MoveList generateLegalMoves(const BoardState& board) {
    MoveList pseudo = generateMoves(board);
    MoveList legal;

    for (auto& move : pseudo.moves) {
        BoardState newBoard = board;
        applyMove(newBoard, move);
        
        // Ensure mover didn't leave king in check
        if (isLegalMoveState(newBoard))
            legal.moves.push_back(move);
    }

    // Move ordering
    legal = orderMoves(legal, board);
    
    return legal;
}
