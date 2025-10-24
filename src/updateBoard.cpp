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

/**
 * Applies a move to the board state, updating piece bitboards and game state.
 * Handles captures, promotions, en passant, and castling rights.
 */
void applyMove(BoardState& board, const Move& move) {
    // Determine side to move
    bool white = board.whiteToMove;

    // Update piece bitboards
    if (white) {
        // White to move
        if (GET_BIT(board.whitePawns, move.from)) {
            // Moving a white pawn
            board.whitePawns &= ~(1ULL << move.from); // Remove from original square
            if (move.promotion != '\0') {
                // Handle promotion
                switch (move.promotion) {
                    case 'Q': board.whiteQueens |= (1ULL << move.to); break;
                    case 'R': board.whiteRooks  |= (1ULL << move.to); break;
                    case 'B': board.whiteBishops|= (1ULL << move.to); break;
                    case 'N': board.whiteKnights|= (1ULL << move.to); break;
                }
            } else {
                board.whitePawns |= (1ULL << move.to); // Move to new square
            }
            // Handle en passant capture
            if (move.isEnPassant) {
                int epCaptureSquare = move.to - 8;
                board.blackPawns &= ~(1ULL << epCaptureSquare); // Remove captured pawn
            }
        }
        // Handle other white pieces similarly...
        if (GET_BIT(board.whiteKnights, move.from)) {
            board.whiteKnights &= ~(1ULL << move.from);
            board.whiteKnights |= (1ULL << move.to);
        } else if (GET_BIT(board.whiteBishops, move.from)) {
            board.whiteBishops &= ~(1ULL << move.from);
            board.whiteBishops |= (1ULL << move.to);
        } else if (GET_BIT(board.whiteRooks, move.from)) {
            board.whiteRooks &= ~(1ULL << move.from);
            board.whiteRooks |= (1ULL << move.to);
        } else if (GET_BIT(board.whiteQueens, move.from)) {
            board.whiteQueens &= ~(1ULL << move.from);
            board.whiteQueens |= (1ULL << move.to);
        } else if (GET_BIT(board.whiteKing, move.from)) {
            board.whiteKing &= ~(1ULL << move.from);
            board.whiteKing |= (1ULL << move.to);
            // Handle castling
            if (move.isCastling) {
                if (move.to == 6) { // Kingside
                    board.whiteRooks &= ~(1ULL << 7);
                    board.whiteRooks |= (1ULL << 5);
                } else if (move.to == 2) { // Queenside
                    board.whiteRooks &= ~(1ULL << 0);
                    board.whiteRooks |= (1ULL << 3);
                }
            }
        }
        
    } else {
        // Black to move
        if (GET_BIT(board.blackPawns, move.from)) {
            // Moving a black pawn
            board.blackPawns &= ~(1ULL << move.from); // Remove from original square
            if (move.promotion != '\0') {
                // Handle promotion
                switch (move.promotion) {
                    case 'q': board.blackQueens |= (1ULL << move.to); break;
                    case 'r': board.blackRooks  |= (1ULL << move.to); break;
                    case 'b': board.blackBishops|= (1ULL << move.to); break;
                    case 'n': board.blackKnights|= (1ULL << move.to); break;
                }
            } else {
                board.blackPawns |= (1ULL << move.to); // Move to new square
            }
            // Handle en passant capture
            if (move.isEnPassant) {
                int epCaptureSquare = move.to + 8;
                board.whitePawns &= ~(1ULL << epCaptureSquare); // Remove captured pawn
            }
        }
        // Handle other black pieces similarly...
        if (GET_BIT(board.blackKnights, move.from)) {
            board.blackKnights &= ~(1ULL << move.from);
            board.blackKnights |= (1ULL << move.to);
        } else if (GET_BIT(board.blackBishops, move.from)) {
            board.blackBishops &= ~(1ULL << move.from);
            board.blackBishops |= (1ULL << move.to);
        } else if (GET_BIT(board.blackRooks, move.from)) {
            board.blackRooks &= ~(1ULL << move.from);
            board.blackRooks |= (1ULL << move.to);
        } else if (GET_BIT(board.blackQueens, move.from)) {
            board.blackQueens &= ~(1ULL << move.from);
            board.blackQueens |= (1ULL << move.to);
        } else if (GET_BIT(board.blackKing, move.from)) {
            board.blackKing &= ~(1ULL << move.from);
            board.blackKing |= (1ULL << move.to);
            // Handle castling
            if (move.isCastling) {
                if (move.to == 62) { // Kingside
                    board.blackRooks &= ~(1ULL << 63);
                    board.blackRooks |= (1ULL << 61);
                } else if (move.to == 58) { // Queenside
                    board.blackRooks &= ~(1ULL << 56);
                    board.blackRooks |= (1ULL << 59);
                }
            }
        }
    }

    // Update game state (castling rights, en passant square)
    updateGameState(board, move);


    // Switch side to move
    board.whiteToMove = !board.whiteToMove;
}   