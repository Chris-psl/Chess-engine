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
        if (GET_BIT(oppPawns, move.to))        oppPawns   &= ~(1ULL << move.to);
        else if (GET_BIT(oppKnights, move.to)) oppKnights &= ~(1ULL << move.to);
        else if (GET_BIT(oppBishops, move.to)) oppBishops &= ~(1ULL << move.to);
        else if (GET_BIT(oppRooks, move.to))   oppRooks   &= ~(1ULL << move.to);
        else if (GET_BIT(oppQueens, move.to))  oppQueens  &= ~(1ULL << move.to);
        else if (GET_BIT(oppKing, move.to))    oppKing    &= ~(1ULL << move.to); // should not happen in legal move
    }

    // === 4) En Passant capture ===
    if (move.isEnPassant) {
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
    if (movedPiece == KING && move.isCastling) {
        if (white) {
            if (move.to == 6) {      // White O-O
                myRooks &= ~(1ULL << 7);
                myRooks |=  (1ULL << 5);
            } else if (move.to == 2) { // White O-O-O
                myRooks &= ~(1ULL << 0);
                myRooks |=  (1ULL << 3);
            }
        } else {
            if (move.to == 62) {      // Black O-O
                myRooks &= ~(1ULL << 63);
                myRooks |=  (1ULL << 61);
            } else if (move.to == 58) { // Black O-O-O
                myRooks &= ~(1ULL << 56);
                myRooks |=  (1ULL << 59);
            }
        }
    }

    // === 7) Game-state bookkeeping (EP square, castling rights, clocks) ===
    updateGameState(board, move);

    // === 8) Switch side ===
    board.whiteToMove = !board.whiteToMove;
}
