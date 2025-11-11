// updateBoard.cpp - Functions to update the board state and apply moves

#include "movegen.h"
#include "utils.h"
#include "zobrist.h"
#include "updateBoard.h"
#include <cassert>


/**
 * Updates the en passant square in the board state based on the last move.
 * If the last move was a two-square pawn advance, sets the en passant square.
 * Otherwise, resets it to -1.
 * 
 * Must be called after making a move to update the board state.
 */
void updateEnPassantSquare(BoardState& board, const Move& move) {
    // Reset en passant square by default
    //board.enPassantSquare = -1;
   
    // Check if the move is a two-square pawn advance
    if (board.whiteToMove) {
        // White to move
        if (GET_BIT(board.whitePawns, move.from) && (move.to - move.from) == 16) {
            board.enPassantSquare = move.from + 8; // Square behind the pawn
            //addMove(move.from, move.to, '\0', false, true, false);
        }else {
            board.enPassantSquare = -1;
        }
    } else {
        // Black to move
        if (GET_BIT(board.blackPawns, move.from) && (move.from - move.to) == 16) {
            board.enPassantSquare = move.from - 8; // Square behind the pawn
            //addMove(move.from, move.to, '\0', false, true, false);
        }else {
            board.enPassantSquare = -1;
        }
    }
}

/**
 * Updates the castling rights in the board state based on the last move.
 * Removes castling rights if the king or rook has moved.
 */
void removeRight(std::string& rights, char c) {
    size_t pos = rights.find(c);
    if (pos != std::string::npos)
        rights.erase(pos, 1);
}

void updateCastlingRights(BoardState& board, const Move& move) {
    bool white = board.whiteToMove;

    if (board.castlingRights == "no_castling") return;
    

    // 1. If the side's king moved — remove both rights
    if ((white && GET_BIT(board.whiteKing, move.from)) || (!white && GET_BIT(board.blackKing, move.from))) {
        if (white) { removeRight(board.castlingRights, 'K'); removeRight(board.castlingRights, 'Q'); }
        else        { removeRight(board.castlingRights, 'k'); removeRight(board.castlingRights, 'q'); }
    }

    // 2. If a rook moved — remove its side’s right
    if ((white && GET_BIT(board.whiteRooks, move.from)) || (!white && GET_BIT(board.blackRooks, move.from))) {
        if (white) {
            if (move.from == 0) removeRight(board.castlingRights, 'Q');
            if (move.from == 7) removeRight(board.castlingRights, 'K');
        } else {
            if (move.from == 56) removeRight(board.castlingRights, 'q');
            if (move.from == 63) removeRight(board.castlingRights, 'k');
        }
    }

    // 3. If an enemy rook was captured — remove that side’s right
    if (move.isCapture) {
        if (white) { // white just moved, black's rook might be gone
            if (move.to == 56) removeRight(board.castlingRights, 'q');
            if (move.to == 63) removeRight(board.castlingRights, 'k');
        } else {
            if (move.to == 0) removeRight(board.castlingRights, 'Q');
            if (move.to == 7) removeRight(board.castlingRights, 'K');
        }
    }

    if(board.castlingRights.empty())
        board.castlingRights = "no_castling";
}

// Function that updates the castling rights and en passant square after a move
void updateGameState(BoardState& board, const Move& move) {
    updateCastlingRights(board, move);
    updateEnPassantSquare(board, move);
}

// Function that applies a move to the board state
// void applyMove(BoardState& board, const Move& move) {
//     bool white = board.whiteToMove;

//     // increment halfmove clock
//     board.halfmoveClock++;

//     // Bitboard refs
//     uint64_t &myPawns   = white ? board.whitePawns   : board.blackPawns;
//     uint64_t &myKnights = white ? board.whiteKnights : board.blackKnights;
//     uint64_t &myBishops = white ? board.whiteBishops : board.blackBishops;
//     uint64_t &myRooks   = white ? board.whiteRooks   : board.blackRooks;
//     uint64_t &myQueens  = white ? board.whiteQueens  : board.blackQueens;
//     uint64_t &myKing    = white ? board.whiteKing    : board.blackKing;

//     uint64_t &oppPawns   = white ? board.blackPawns   : board.whitePawns;
//     uint64_t &oppKnights = white ? board.blackKnights : board.whiteKnights;
//     uint64_t &oppBishops = white ? board.blackBishops : board.whiteBishops;
//     uint64_t &oppRooks   = white ? board.blackRooks   : board.whiteRooks;
//     uint64_t &oppQueens  = white ? board.blackQueens  : board.whiteQueens;
//     uint64_t &oppKing    = white ? board.blackKing    : board.whiteKing;

//     // === 1) Identify moving piece before removal ===
//     enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING } movedPiece;
//     if (GET_BIT(myPawns, move.from)) movedPiece = PAWN;
//     else if (GET_BIT(myKnights, move.from)) movedPiece = KNIGHT;
//     else if (GET_BIT(myBishops, move.from)) movedPiece = BISHOP;
//     else if (GET_BIT(myRooks, move.from)) movedPiece = ROOK;
//     else if (GET_BIT(myQueens, move.from)) movedPiece = QUEEN;
//     else movedPiece = KING; // must be king

//     // === 2) Remove piece from source square ===
//     switch (movedPiece) {
//         case PAWN:   myPawns   &= ~(1ULL << move.from); break;
//         case KNIGHT: myKnights &= ~(1ULL << move.from); break;
//         case BISHOP: myBishops &= ~(1ULL << move.from); break;
//         case ROOK:   myRooks   &= ~(1ULL << move.from); break;
//         case QUEEN:  myQueens  &= ~(1ULL << move.from); break;
//         case KING:   myKing    &= ~(1ULL << move.from); break;
//     }

//     // === 3) Handle capture (non-EP) ===
//     if (move.isCapture && !move.isEnPassant) {
//         // Reset halfmove clock on capture
//         board.halfmoveClock = 0;

//         if (GET_BIT(oppPawns, move.to))        oppPawns   &= ~(1ULL << move.to);
//         else if (GET_BIT(oppKnights, move.to)) oppKnights &= ~(1ULL << move.to);
//         else if (GET_BIT(oppBishops, move.to)) oppBishops &= ~(1ULL << move.to);
//         else if (GET_BIT(oppRooks, move.to))   oppRooks   &= ~(1ULL << move.to);
//         else if (GET_BIT(oppQueens, move.to))  oppQueens  &= ~(1ULL << move.to);
//         else if (GET_BIT(oppKing, move.to))    oppKing    &= ~(1ULL << move.to); // should not happen in legal move
//     }

//     // === 4) En Passant capture ===
//     if (move.isEnPassant) {
//         // Reset halfmove clock on capture
//         board.halfmoveClock = 0;

//         int capSq = white ? move.to - 8 : move.to + 8;
//         oppPawns &= ~(1ULL << capSq);
//     }

//     // === 5) Place moved piece to target (promotion supported) ===
//     if (movedPiece == PAWN && move.promotion) {
//         switch (move.promotion) {
//             case 'Q': myQueens  |= (1ULL << move.to); break;
//             case 'R': myRooks   |= (1ULL << move.to); break;
//             case 'B': myBishops |= (1ULL << move.to); break;
//             case 'N': myKnights |= (1ULL << move.to); break;
//         }
//     } else {
//         switch (movedPiece) {
//             case PAWN:   myPawns   |= (1ULL << move.to); break;
//             case KNIGHT: myKnights |= (1ULL << move.to); break;
//             case BISHOP: myBishops |= (1ULL << move.to); break;
//             case ROOK:   myRooks   |= (1ULL << move.to); break;
//             case QUEEN:  myQueens  |= (1ULL << move.to); break;
//             case KING:   myKing    |= (1ULL << move.to); break;
//         }
//     }

//      // === 6) Castling rook movement ===
//     if (movedPiece == KING && (move.isCastling || abs(move.to - move.from) == 2)) {
//         // Determine castling by the king moving two squares.
//         // Use explicit squares so there's no ambiguity.
//         const int WHITE_KING_FROM = 4;  // e1
//         const int BLACK_KING_FROM = 60; // e8
//         const int WHITE_KINGSIDE_ROOK_FROM = 7;  // h1
//         const int WHITE_QUEENSIDE_ROOK_FROM = 0; // a1
//         const int BLACK_KINGSIDE_ROOK_FROM = 63; // h8
//         const int BLACK_QUEENSIDE_ROOK_FROM = 56; // a8

//         if (white) {
//             // White king-side: e1 -> g1 (4 -> 6)
//             if (move.to == 6 && move.from == WHITE_KING_FROM) {
//                 // Ensure rook exists before moving (defensive)
//                 if (GET_BIT(myRooks, WHITE_KINGSIDE_ROOK_FROM)) {
//                     myRooks &= ~(1ULL << WHITE_KINGSIDE_ROOK_FROM);
//                     myRooks |=  (1ULL << 5); // h1 -> f1 (7 -> 5)
//                 } else {
//                     // Defensive fallback: try to find rook on expected file rank
//                     // (optional) -- don't crash; just attempt to move rook from 7->5 anyway
//                     myRooks &= ~(1ULL << WHITE_KINGSIDE_ROOK_FROM);
//                     myRooks |=  (1ULL << 5);
//                 }
//             }
//             // White queen-side: e1 -> c1 (4 -> 2)
//             else if (move.to == 2 && move.from == WHITE_KING_FROM) {
//                 if (GET_BIT(myRooks, WHITE_QUEENSIDE_ROOK_FROM)) {
//                     myRooks &= ~(1ULL << WHITE_QUEENSIDE_ROOK_FROM);
//                     myRooks |=  (1ULL << 3); // a1 -> d1 (0 -> 3)
//                 } else {
//                     myRooks &= ~(1ULL << WHITE_QUEENSIDE_ROOK_FROM);
//                     myRooks |=  (1ULL << 3);
//                 }
//             }
//         } else {
//             // Black king-side: e8 -> g8 (60 -> 62)
//             if (move.to == 62 && move.from == BLACK_KING_FROM) {
//                 if (GET_BIT(myRooks, BLACK_KINGSIDE_ROOK_FROM)) {
//                     myRooks &= ~(1ULL << BLACK_KINGSIDE_ROOK_FROM);
//                     myRooks |=  (1ULL << 61); // h8 -> f8 (63 -> 61)
//                 } else {
//                     myRooks &= ~(1ULL << BLACK_KINGSIDE_ROOK_FROM);
//                     myRooks |=  (1ULL << 61);
//                 }
//             }
//             // Black queen-side: e8 -> c8 (60 -> 58)
//             else if (move.to == 58 && move.from == BLACK_KING_FROM) {
//                 if (GET_BIT(myRooks, BLACK_QUEENSIDE_ROOK_FROM)) {
//                     myRooks &= ~(1ULL << BLACK_QUEENSIDE_ROOK_FROM);
//                     myRooks |=  (1ULL << 59); // a8 -> d8 (56 -> 59)
//                 } else {
//                     myRooks &= ~(1ULL << BLACK_QUEENSIDE_ROOK_FROM);
//                     myRooks |=  (1ULL << 59);
//                 }
//             }
//         }
//     }

//     // === 7) Game-state bookkeeping (EP square, castling rights, clocks) ===
//     updateGameState(board, move);

//     // === 8) Switch side ===
//     board.whiteToMove = !board.whiteToMove;
// }



// helper: map (color, piece type enum used earlier) -> zobrist index
static inline int pieceIndex(bool white, int pieceEnum) {
    // pieceEnum: PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING (as used in your applyMove)
    // ordering: white: 0..5, black: 6..11
    return (white ? 0 : 6) + pieceEnum;
}

void applyMove(BoardState& board, const Move& move) {
    bool white = board.whiteToMove;

    // Save previous state for zobrist updates
    int oldCastlingMask = castlingMask(board.castlingRights);
    int oldEnPassant = board.enPassantSquare; // -1 if none

    // increment halfmove clock (bookkeeping)
    board.halfmoveClock++;

    // Bitboard refs (same as before)
    uint64_t &myPawns   = white ? board.whitePawns   : board.blackPawns;
    uint64_t &myKnights = white ? board.whiteKnights : board.blackKnights;
    uint64_t &myBishops = white ? board.whiteBishops : board.blackBishops;
    uint64_t &myRooks   = white ? board.whiteRooks   : board.blackRooks;
    uint64_t &myQueens  = white ? board.whiteQueens  : board.blackQueens;
    uint64_t &myKing    = white ? board.whiteKing    : board.blackKing;

    uint64_t &oppPawns   = white ? board.blackPawns   : board.whitePawns;
    uint64_t &oppKnights = white ? board.blackKnights : board.whiteKnights;
    uint64_t &oppBishops = white ? board.blackBishops : board.whiteBishops;
    uint64_t &oppRooks   = white ? board.blackRooks   : board.whiteRooks;
    uint64_t &oppQueens  = white ? board.blackQueens  : board.whiteQueens;
    uint64_t &oppKing    = white ? board.blackKing    : board.whiteKing;

    // === Identify moving piece before removal ===
    enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING } movedPiece;
    if (GET_BIT(myPawns, move.from)) movedPiece = PAWN;
    else if (GET_BIT(myKnights, move.from)) movedPiece = KNIGHT;
    else if (GET_BIT(myBishops, move.from)) movedPiece = BISHOP;
    else if (GET_BIT(myRooks, move.from)) movedPiece = ROOK;
    else if (GET_BIT(myQueens, move.from)) movedPiece = QUEEN;
    else movedPiece = KING; // must be king

    // === Zobrist: XOR out moving piece from 'from' square ===
    board.zobristKey ^= zobristTable[pieceIndex(white, movedPiece)][move.from];

    // === Remove piece from source square (bitboards) ===
    switch (movedPiece) {
        case PAWN:   myPawns   &= ~(1ULL << move.from); break;
        case KNIGHT: myKnights &= ~(1ULL << move.from); break;
        case BISHOP: myBishops &= ~(1ULL << move.from); break;
        case ROOK:   myRooks   &= ~(1ULL << move.from); break;
        case QUEEN:  myQueens  &= ~(1ULL << move.from); break;
        case KING:   myKing    &= ~(1ULL << move.from); break;
    }

    // === Handle capture (non-EP) and update zobrist for captured piece ===
    if (move.isCapture && !move.isEnPassant) {
        // Reset halfmove clock on capture
        board.halfmoveClock = 0;

        if (GET_BIT(oppPawns, move.to)) {
            // capture pawn
            // xor out captured pawn
            board.zobristKey ^= zobristTable[pieceIndex(!white, PAWN)][move.to];
            oppPawns   &= ~(1ULL << move.to);
        }
        else if (GET_BIT(oppKnights, move.to)) {
            board.zobristKey ^= zobristTable[pieceIndex(!white, KNIGHT)][move.to];
            oppKnights &= ~(1ULL << move.to);
        }
        else if (GET_BIT(oppBishops, move.to)) {
            board.zobristKey ^= zobristTable[pieceIndex(!white, BISHOP)][move.to];
            oppBishops &= ~(1ULL << move.to);
        }
        else if (GET_BIT(oppRooks, move.to)) {
            board.zobristKey ^= zobristTable[pieceIndex(!white, ROOK)][move.to];
            oppRooks   &= ~(1ULL << move.to);
        }
        else if (GET_BIT(oppQueens, move.to)) {
            board.zobristKey ^= zobristTable[pieceIndex(!white, QUEEN)][move.to];
            oppQueens  &= ~(1ULL << move.to);
        }
        else if (GET_BIT(oppKing, move.to)) {
            board.zobristKey ^= zobristTable[pieceIndex(!white, KING)][move.to];
            oppKing    &= ~(1ULL << move.to); // should not happen in legal move
        }
    }

    // === En Passant capture (special square) ===
    if (move.isEnPassant) {
        // Reset halfmove clock on capture
        board.halfmoveClock = 0;

        int capSq = white ? move.to - 8 : move.to + 8;
        // xor out captured pawn on capSq
        board.zobristKey ^= zobristTable[pieceIndex(!white, PAWN)][capSq];
        oppPawns &= ~(1ULL << capSq);
    }

    // === Place moved piece to target (promotion supported) and update zobrist ===
    if (movedPiece == PAWN && move.promotion) {
        // For promotion: pawn removed from 'from' already xor'ed out above.
        // XOR in promoted piece on 'to' square
        switch (move.promotion) {
            case 'Q':
                myQueens  |= (1ULL << move.to);
                board.zobristKey ^= zobristTable[pieceIndex(white, QUEEN)][move.to];
                break;
            case 'R':
                myRooks   |= (1ULL << move.to);
                board.zobristKey ^= zobristTable[pieceIndex(white, ROOK)][move.to];
                break;
            case 'B':
                myBishops |= (1ULL << move.to);
                board.zobristKey ^= zobristTable[pieceIndex(white, BISHOP)][move.to];
                break;
            case 'N':
                myKnights |= (1ULL << move.to);
                board.zobristKey ^= zobristTable[pieceIndex(white, KNIGHT)][move.to];
                break;
            default:
                // Unknown promotion piece - fallback to queen
                myQueens  |= (1ULL << move.to);
                board.zobristKey ^= zobristTable[pieceIndex(white, QUEEN)][move.to];
                break;
        }
    } else {
        // Normal non-promotion move: XOR in moving piece at 'to'
        switch (movedPiece) {
            case PAWN:
                myPawns   |= (1ULL << move.to);
                board.zobristKey ^= zobristTable[pieceIndex(white, PAWN)][move.to];
                break;
            case KNIGHT:
                myKnights |= (1ULL << move.to);
                board.zobristKey ^= zobristTable[pieceIndex(white, KNIGHT)][move.to];
                break;
            case BISHOP:
                myBishops |= (1ULL << move.to);
                board.zobristKey ^= zobristTable[pieceIndex(white, BISHOP)][move.to];
                break;
            case ROOK:
                myRooks   |= (1ULL << move.to);
                board.zobristKey ^= zobristTable[pieceIndex(white, ROOK)][move.to];
                break;
            case QUEEN:
                myQueens  |= (1ULL << move.to);
                board.zobristKey ^= zobristTable[pieceIndex(white, QUEEN)][move.to];
                break;
            case KING:
                myKing    |= (1ULL << move.to);
                board.zobristKey ^= zobristTable[pieceIndex(white, KING)][move.to];
                break;
        }
    }

    // === Castling rook movement (bitboards + zobrist updates) ===
    if (movedPiece == KING && (move.isCastling || abs(move.to - move.from) == 2)) {
        const int WHITE_KING_FROM = 4;  // e1
        const int BLACK_KING_FROM = 60; // e8
        const int WHITE_KINGSIDE_ROOK_FROM = 7;  // h1
        const int WHITE_QUEENSIDE_ROOK_FROM = 0; // a1
        const int BLACK_KINGSIDE_ROOK_FROM = 63; // h8
        const int BLACK_QUEENSIDE_ROOK_FROM = 56; // a8

        if (white) {
            // White king-side: e1 -> g1 (4 -> 6); rook 7->5
            if (move.to == 6 && move.from == WHITE_KING_FROM) {
                // XOR out rook from 7, xor in rook at 5
                board.zobristKey ^= zobristTable[pieceIndex(white, ROOK)][WHITE_KINGSIDE_ROOK_FROM];
                board.zobristKey ^= zobristTable[pieceIndex(white, ROOK)][5];

                // bitboard adjust
                myRooks &= ~(1ULL << WHITE_KINGSIDE_ROOK_FROM);
                myRooks |=  (1ULL << 5);
            }
            // White queen-side: e1 -> c1 (4 -> 2); rook 0->3
            else if (move.to == 2 && move.from == WHITE_KING_FROM) {
                board.zobristKey ^= zobristTable[pieceIndex(white, ROOK)][WHITE_QUEENSIDE_ROOK_FROM];
                board.zobristKey ^= zobristTable[pieceIndex(white, ROOK)][3];

                myRooks &= ~(1ULL << WHITE_QUEENSIDE_ROOK_FROM);
                myRooks |=  (1ULL << 3);
            }
        } else {
            // Black king-side: e8 -> g8 (60 -> 62); rook 63->61
            if (move.to == 62 && move.from == BLACK_KING_FROM) {
                board.zobristKey ^= zobristTable[pieceIndex(white, ROOK)][BLACK_KINGSIDE_ROOK_FROM];
                board.zobristKey ^= zobristTable[pieceIndex(white, ROOK)][61];

                myRooks &= ~(1ULL << BLACK_KINGSIDE_ROOK_FROM);
                myRooks |=  (1ULL << 61);
            }
            // Black queen-side: e8 -> c8 (60 -> 58); rook 56->59
            else if (move.to == 58 && move.from == BLACK_KING_FROM) {
                board.zobristKey ^= zobristTable[pieceIndex(white, ROOK)][BLACK_QUEENSIDE_ROOK_FROM];
                board.zobristKey ^= zobristTable[pieceIndex(white, ROOK)][59];

                myRooks &= ~(1ULL << BLACK_QUEENSIDE_ROOK_FROM);
                myRooks |=  (1ULL << 59);
            }
        }
    }

    // === Zobrist: XOR out old castling & old en-passant keys before updateGameState ===
    board.zobristKey ^= zobristCastling[oldCastlingMask];
    if (oldEnPassant != -1) {
        int oldFile = oldEnPassant % 8;
        assert(oldFile >= 0 && oldFile < 8);
        board.zobristKey ^= zobristEnPassant[oldFile];
    }

    // === 7) Game-state bookkeeping (EP square, castling rights, clocks) ===
    updateGameState(board, move);

    // === Zobrist: XOR in new castling & new en-passant keys after updateGameState ===
    int newCastlingMask = castlingMask(board.castlingRights);
    board.zobristKey ^= zobristCastling[newCastlingMask];

    int newEnPassant = board.enPassantSquare;
    if (newEnPassant != -1) {
        int newFile = newEnPassant % 8;
        assert(newFile >= 0 && newFile < 8);
        board.zobristKey ^= zobristEnPassant[newFile];
    }

    // === 8) Toggle side to move in zobrist and switch side ===
    board.zobristKey ^= zobristWhiteToMove;
    board.whiteToMove = !board.whiteToMove;
}
