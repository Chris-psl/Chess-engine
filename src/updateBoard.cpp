// updateBoard.cpp - Functions to update the board state and apply moves

#include "movegen.h"
#include "utils.h"


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

void applyMove(BoardState& board, const Move& move) {
    bool white = board.whiteToMove;

    // increment halfmove clock
    board.halfmoveClock++;

    // Bitboard refs
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

    // === 1) Identify moving piece before removal ===
    enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING } movedPiece;
    if (GET_BIT(myPawns, move.from)) movedPiece = PAWN;
    else if (GET_BIT(myKnights, move.from)) movedPiece = KNIGHT;
    else if (GET_BIT(myBishops, move.from)) movedPiece = BISHOP;
    else if (GET_BIT(myRooks, move.from)) movedPiece = ROOK;
    else if (GET_BIT(myQueens, move.from)) movedPiece = QUEEN;
    else movedPiece = KING; // must be king

    // === 2) Remove piece from source square ===
    switch (movedPiece) {
        case PAWN:   myPawns   &= ~(1ULL << move.from); break;
        case KNIGHT: myKnights &= ~(1ULL << move.from); break;
        case BISHOP: myBishops &= ~(1ULL << move.from); break;
        case ROOK:   myRooks   &= ~(1ULL << move.from); break;
        case QUEEN:  myQueens  &= ~(1ULL << move.from); break;
        case KING:   myKing    &= ~(1ULL << move.from); break;
    }

    // === 3) Handle capture (non-EP) ===
    if (move.isCapture && !move.isEnPassant) {
        // Reset halfmove clock on capture
        board.halfmoveClock = 0;

        if (GET_BIT(oppPawns, move.to))        oppPawns   &= ~(1ULL << move.to);
        else if (GET_BIT(oppKnights, move.to)) oppKnights &= ~(1ULL << move.to);
        else if (GET_BIT(oppBishops, move.to)) oppBishops &= ~(1ULL << move.to);
        else if (GET_BIT(oppRooks, move.to))   oppRooks   &= ~(1ULL << move.to);
        else if (GET_BIT(oppQueens, move.to))  oppQueens  &= ~(1ULL << move.to);
        else if (GET_BIT(oppKing, move.to))    oppKing    &= ~(1ULL << move.to); // should not happen in legal move
    }

    // === 4) En Passant capture ===
    if (move.isEnPassant) {
        // Reset halfmove clock on capture
        board.halfmoveClock = 0;

        int capSq = white ? move.to - 8 : move.to + 8;
        oppPawns &= ~(1ULL << capSq);
    }

    // === 5) Place moved piece to target (promotion supported) ===
    if (movedPiece == PAWN && move.promotion) {
        switch (move.promotion) {
            case 'Q': myQueens  |= (1ULL << move.to); break;
            case 'R': myRooks   |= (1ULL << move.to); break;
            case 'B': myBishops |= (1ULL << move.to); break;
            case 'N': myKnights |= (1ULL << move.to); break;
        }
    } else {
        switch (movedPiece) {
            case PAWN:   myPawns   |= (1ULL << move.to); break;
            case KNIGHT: myKnights |= (1ULL << move.to); break;
            case BISHOP: myBishops |= (1ULL << move.to); break;
            case ROOK:   myRooks   |= (1ULL << move.to); break;
            case QUEEN:  myQueens  |= (1ULL << move.to); break;
            case KING:   myKing    |= (1ULL << move.to); break;
        }
    }

     // === 6) Castling rook movement ===
    if (movedPiece == KING && (move.isCastling || abs(move.to - move.from) == 2)) {
        // Determine castling by the king moving two squares.
        // Use explicit squares so there's no ambiguity.
        const int WHITE_KING_FROM = 4;  // e1
        const int BLACK_KING_FROM = 60; // e8
        const int WHITE_KINGSIDE_ROOK_FROM = 7;  // h1
        const int WHITE_QUEENSIDE_ROOK_FROM = 0; // a1
        const int BLACK_KINGSIDE_ROOK_FROM = 63; // h8
        const int BLACK_QUEENSIDE_ROOK_FROM = 56; // a8

        if (white) {
            // White king-side: e1 -> g1 (4 -> 6)
            if (move.to == 6 && move.from == WHITE_KING_FROM) {
                // Ensure rook exists before moving (defensive)
                if (GET_BIT(myRooks, WHITE_KINGSIDE_ROOK_FROM)) {
                    myRooks &= ~(1ULL << WHITE_KINGSIDE_ROOK_FROM);
                    myRooks |=  (1ULL << 5); // h1 -> f1 (7 -> 5)
                } else {
                    // Defensive fallback: try to find rook on expected file rank
                    // (optional) -- don't crash; just attempt to move rook from 7->5 anyway
                    myRooks &= ~(1ULL << WHITE_KINGSIDE_ROOK_FROM);
                    myRooks |=  (1ULL << 5);
                }
            }
            // White queen-side: e1 -> c1 (4 -> 2)
            else if (move.to == 2 && move.from == WHITE_KING_FROM) {
                if (GET_BIT(myRooks, WHITE_QUEENSIDE_ROOK_FROM)) {
                    myRooks &= ~(1ULL << WHITE_QUEENSIDE_ROOK_FROM);
                    myRooks |=  (1ULL << 3); // a1 -> d1 (0 -> 3)
                } else {
                    myRooks &= ~(1ULL << WHITE_QUEENSIDE_ROOK_FROM);
                    myRooks |=  (1ULL << 3);
                }
            }
        } else {
            // Black king-side: e8 -> g8 (60 -> 62)
            if (move.to == 62 && move.from == BLACK_KING_FROM) {
                if (GET_BIT(myRooks, BLACK_KINGSIDE_ROOK_FROM)) {
                    myRooks &= ~(1ULL << BLACK_KINGSIDE_ROOK_FROM);
                    myRooks |=  (1ULL << 61); // h8 -> f8 (63 -> 61)
                } else {
                    myRooks &= ~(1ULL << BLACK_KINGSIDE_ROOK_FROM);
                    myRooks |=  (1ULL << 61);
                }
            }
            // Black queen-side: e8 -> c8 (60 -> 58)
            else if (move.to == 58 && move.from == BLACK_KING_FROM) {
                if (GET_BIT(myRooks, BLACK_QUEENSIDE_ROOK_FROM)) {
                    myRooks &= ~(1ULL << BLACK_QUEENSIDE_ROOK_FROM);
                    myRooks |=  (1ULL << 59); // a8 -> d8 (56 -> 59)
                } else {
                    myRooks &= ~(1ULL << BLACK_QUEENSIDE_ROOK_FROM);
                    myRooks |=  (1ULL << 59);
                }
            }
        }
    }

    // === 7) Game-state bookkeeping (EP square, castling rights, clocks) ===
    updateGameState(board, move);

    // === 8) Switch side ===
    board.whiteToMove = !board.whiteToMove;
}


// ============================================================================
// Second implementation
// ============================================================================

// // updateBoard.cpp - Functions to update the board state and apply moves

// #include "movegen.h"
// #include "utils.h"


// /**
//  * Updates the en passant square in the board state based on the last move.
//  * If the last move was a two-square pawn advance, sets the en passant square.
//  * Otherwise, resets it to -1.
//  * 
//  * Must be called after making a move to update the board state.
//  */
// void updateEnPassantSquare(BoardState& board, const Move& move) {
//     // Reset en passant square by default
//     //board.enPassantSquare = -1;
   
//     // Check if the move is a two-square pawn advance
//     if (board.whiteToMove) {
//         // White to move
//         if (GET_BIT(board.whitePawns, move.from) && (move.to - move.from) == 16) {
//             board.enPassantSquare = move.from + 8; // Square behind the pawn
//             //addMove(move.from, move.to, '\0', false, true, false);
//         }else {
//             board.enPassantSquare = -1;
//         }
//     } else {
//         // Black to move
//         if (GET_BIT(board.blackPawns, move.from) && (move.from - move.to) == 16) {
//             board.enPassantSquare = move.from - 8; // Square behind the pawn
//             //addMove(move.from, move.to, '\0', false, true, false);
//         }else {
//             board.enPassantSquare = -1;
//         }
//     }
// }

// /**
//  * Updates the castling rights in the board state based on the last move.
//  * Removes castling rights if the king or rook has moved.
//  */
// void removeRight(std::string& rights, char c) {
//     size_t pos = rights.find(c);
//     if (pos != std::string::npos)
//         rights.erase(pos, 1);
// }

// void updateCastlingRights(BoardState& board, const Move& move) {
//     bool white = board.whiteToMove;

//     if (board.castlingRights == "no_castling") return;
    

//     // 1. If the side's king moved — remove both rights
//     if ((white && GET_BIT(board.whiteKing, move.from)) || (!white && GET_BIT(board.blackKing, move.from))) {
//         if (white) { removeRight(board.castlingRights, 'K'); removeRight(board.castlingRights, 'Q'); }
//         else        { removeRight(board.castlingRights, 'k'); removeRight(board.castlingRights, 'q'); }
//     }

//     // 2. If a rook moved — remove its side’s right
//     if ((white && GET_BIT(board.whiteRooks, move.from)) || (!white && GET_BIT(board.blackRooks, move.from))) {
//         if (white) {
//             if (move.from == 0) removeRight(board.castlingRights, 'Q');
//             if (move.from == 7) removeRight(board.castlingRights, 'K');
//         } else {
//             if (move.from == 56) removeRight(board.castlingRights, 'q');
//             if (move.from == 63) removeRight(board.castlingRights, 'k');
//         }
//     }

//     // 3. If an enemy rook was captured — remove that side’s right
//     if (move.isCapture) {
//         if (white) { // white just moved, black's rook might be gone
//             if (move.to == 56) removeRight(board.castlingRights, 'q');
//             if (move.to == 63) removeRight(board.castlingRights, 'k');
//         } else {
//             if (move.to == 0) removeRight(board.castlingRights, 'Q');
//             if (move.to == 7) removeRight(board.castlingRights, 'K');
//         }
//     }

//     if(board.castlingRights.empty())
//         board.castlingRights = "no_castling";
// }

// // Function that updates the castling rights and en passant square after a move
// void updateGameState(BoardState& board, const Move& move) {
//     updateCastlingRights(board, move);
//     updateEnPassantSquare(board, move);
// }

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

//     // === 6) Castling rook movement ===
//     if (movedPiece == KING && move.isCastling) {
//         if (white) {
//             if (move.to == 6) {      // White O-O
//                 myRooks &= ~(1ULL << 7);
//                 myRooks |=  (1ULL << 5);
//             } else if (move.to == 2) { // White O-O-O
//                 myRooks &= ~(1ULL << 0);
//                 myRooks |=  (1ULL << 3);
//             }
//         } else {
//             if (move.to == 62) {      // Black O-O
//                 myRooks &= ~(1ULL << 63);
//                 myRooks |=  (1ULL << 61);
//             } else if (move.to == 58) { // Black O-O-O
//                 myRooks &= ~(1ULL << 56);
//                 myRooks |=  (1ULL << 59);
//             }
//         }
//     }

//     // === 7) Game-state bookkeeping (EP square, castling rights, clocks) ===
//     updateGameState(board, move);

//     // === 8) Switch side ===
//     board.whiteToMove = !board.whiteToMove;
// }


// ============================================================================
// with zobrist hashing
// ============================================================================

// void applyMove(BoardState& board, const Move& move) {
//     bool white = board.whiteToMove;

//     // increment halfmove clock
//     board.halfmoveClock++;

//     // Bitboard references
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

//     // === 1) Identify moving piece ===
//     enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING } movedPiece;
//     if (GET_BIT(myPawns, move.from)) movedPiece = PAWN;
//     else if (GET_BIT(myKnights, move.from)) movedPiece = KNIGHT;
//     else if (GET_BIT(myBishops, move.from)) movedPiece = BISHOP;
//     else if (GET_BIT(myRooks, move.from)) movedPiece = ROOK;
//     else if (GET_BIT(myQueens, move.from)) movedPiece = QUEEN;
//     else movedPiece = KING;

//     int movedIndex = (white ? 0 : 6) + movedPiece;

//     // 🔹 Zobrist: remove moving piece from source
//     board.zobristKey ^= zobristTable[movedIndex][move.from];

//     // === 2) Remove piece from source ===
//     switch (movedPiece) {
//         case PAWN:   myPawns   &= ~(1ULL << move.from); break;
//         case KNIGHT: myKnights &= ~(1ULL << move.from); break;
//         case BISHOP: myBishops &= ~(1ULL << move.from); break;
//         case ROOK:   myRooks   &= ~(1ULL << move.from); break;
//         case QUEEN:  myQueens  &= ~(1ULL << move.from); break;
//         case KING:   myKing    &= ~(1ULL << move.from); break;
//     }

//     // === 3) Handle capture (non-en-passant) ===
//     if (move.isCapture && !move.isEnPassant) {
//         board.halfmoveClock = 0;

//         int capturedIndex = -1;
//         if (GET_BIT(oppPawns, move.to)) { oppPawns &= ~(1ULL << move.to); capturedIndex = (white ? 6 : 0) + PAWN; }
//         else if (GET_BIT(oppKnights, move.to)) { oppKnights &= ~(1ULL << move.to); capturedIndex = (white ? 6 : 0) + KNIGHT; }
//         else if (GET_BIT(oppBishops, move.to)) { oppBishops &= ~(1ULL << move.to); capturedIndex = (white ? 6 : 0) + BISHOP; }
//         else if (GET_BIT(oppRooks, move.to)) { oppRooks &= ~(1ULL << move.to); capturedIndex = (white ? 6 : 0) + ROOK; }
//         else if (GET_BIT(oppQueens, move.to)) { oppQueens &= ~(1ULL << move.to); capturedIndex = (white ? 6 : 0) + QUEEN; }
//         else if (GET_BIT(oppKing, move.to)) { oppKing &= ~(1ULL << move.to); capturedIndex = (white ? 6 : 0) + KING; }

//         if (capturedIndex != -1)
//             board.zobristKey ^= zobristTable[capturedIndex][move.to];
//     }

//     // === 4) En Passant ===
//     if (move.isEnPassant) {
//         board.halfmoveClock = 0;
//         int capSq = white ? move.to - 8 : move.to + 8;
//         oppPawns &= ~(1ULL << capSq);

//         int capIndex = (white ? 6 : 0) + PAWN;
//         board.zobristKey ^= zobristTable[capIndex][capSq];
//     }

//     // === 5) Place moved piece (promotion) ===
//     if (movedPiece == PAWN && move.promotion) {
//         // Remove pawn at destination (if promotion)
//         board.zobristKey ^= zobristTable[movedIndex][move.to];

//         int promoIndex = (white ? 0 : 6);
//         switch (move.promotion) {
//             case 'Q': myQueens  |= 1ULL << move.to; promoIndex += QUEEN; break;
//             case 'R': myRooks   |= 1ULL << move.to; promoIndex += ROOK; break;
//             case 'B': myBishops |= 1ULL << move.to; promoIndex += BISHOP; break;
//             case 'N': myKnights |= 1ULL << move.to; promoIndex += KNIGHT; break;
//         }
//         board.zobristKey ^= zobristTable[promoIndex][move.to];
//     } else {
//         switch (movedPiece) {
//             case PAWN:   myPawns   |= 1ULL << move.to; break;
//             case KNIGHT: myKnights |= 1ULL << move.to; break;
//             case BISHOP: myBishops |= 1ULL << move.to; break;
//             case ROOK:   myRooks   |= 1ULL << move.to; break;
//             case QUEEN:  myQueens  |= 1ULL << move.to; break;
//             case KING:   myKing    |= 1ULL << move.to; break;
//         }
//         board.zobristKey ^= zobristTable[movedIndex][move.to];
//     }

//     // === 6) Castling rook move ===
//     if (movedPiece == KING && (move.isCastling || abs(move.to - move.from) == 2)) {
//         int rookIndex = (white ? 0 : 6) + ROOK;
//         if (white && move.to == 6) { myRooks &= ~(1ULL << 7); myRooks |= 1ULL << 5; board.zobristKey ^= zobristTable[rookIndex][7] ^ zobristTable[rookIndex][5]; }
//         else if (white && move.to == 2) { myRooks &= ~(1ULL << 0); myRooks |= 1ULL << 3; board.zobristKey ^= zobristTable[rookIndex][0] ^ zobristTable[rookIndex][3]; }
//         else if (!white && move.to == 62) { myRooks &= ~(1ULL << 63); myRooks |= 1ULL << 61; board.zobristKey ^= zobristTable[rookIndex][63] ^ zobristTable[rookIndex][61]; }
//         else if (!white && move.to == 58) { myRooks &= ~(1ULL << 56); myRooks |= 1ULL << 59; board.zobristKey ^= zobristTable[rookIndex][56] ^ zobristTable[rookIndex][59]; }
//     }

//     // === 7) Zobrist: castling/en passant before updating game state ===
//     int oldCastleMask = castlingMask(board.castlingRights);
//     if (board.enPassantSquare != -1) board.zobristKey ^= zobristEnPassant[board.enPassantSquare % 8];

//     updateGameState(board, move);

//     int newCastleMask = castlingMask(board.castlingRights);
//     if (oldCastleMask != newCastleMask) {
//         board.zobristKey ^= zobristCastling[oldCastleMask] ^ zobristCastling[newCastleMask];
//     }
//     if (board.enPassantSquare != -1) board.zobristKey ^= zobristEnPassant[board.enPassantSquare % 8];

//     // === 8) Switch side ===
//     board.whiteToMove = !board.whiteToMove;
//     board.zobristKey ^= zobristWhiteToMove;
// }