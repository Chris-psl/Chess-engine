// This is a file that contains functions that generate legal moves for chess pieces based on the current board state.
/*
We will represent moves as strings in algebraic notation, e.g., "e4" for a pawn moving from e2 to e4, e.g. "Nf3" for knight from g1 to f3.
For captures we can use "exf5" to indicate a piece moving to f5 and capturing an opponent's piece there.

If two pieces of the same type can move to the same square, we can disambiguate using the file or rank of the moving piece,
e.g., "Nbd2" (knight from b-file moves to d2) or "R1e2" (rook from rank 1 moves to e2).

Castling:   King-side castling: O-O
            Queen-side castling: O-O-O

Pawn Promotion:             e8=Q → pawn moves to e8 and promotes to queen
                            g1=N → pawn moves to g1 and promotes to knight

Captures with promotion:    fxg8=Q → pawn captures on g8 and promotes to queen

Check(+) :                  Qh5+ → queen moves to h5, giving check

Checkmate(#) :              Qh7# → queen moves to h7, giving checkmate
*/


#include "utils.h"
#include <vector>


// Function that generates all possible legal moves for a bishop at a given position
std::vector<std::string> generateBishopMoves(const BoardState& board, char pieceType, int position){
    std::vector<std::string> moves;
    // Implementation of bishop move generation logic goes here
    return moves;
}

// Function that generates all possible en-passant moves for pawns
std::vector<std::string> generateEnPassantMoves(const BoardState& board, char pieceType, int position){
    std::vector<std::string> moves;
    // Implementation of en-passant move generation logic goes here
    return moves;
}

// Function that generates all possible legal moves for a pawn at a given position
std::vector<std::string> generatePawnMoves(const BoardState& board, char pieceType, int position){
    std::vector<std::string> moves;
    // Implementation of pawn move generation logic goes here   

    return moves;
}

// Function that generates all possible legal moves for a rook at a given position
std::vector<std::string> generateRookMoves(const BoardState& board, char pieceType, int position){
    std::vector<std::string> moves;
    // Implementation of rook move generation logic goes here
    return moves;
}

// Function that generates all possible legal moves for a knight at a given position
std::vector<std::string> generateKnightMoves(const BoardState& board, char pieceType, int position){
    std::vector<std::string> moves;
    // Implementation of knight move generation logic goes here
    return moves;
}

// Function that generates all possible legal moves for a queen at a given position
std::vector<std::string> generateQueenMoves(const BoardState& board, char pieceType, int position){
    std::vector<std::string> moves;
    // Implementation of queen move generation logic goes here
    return moves;
}

// Function that generates all possible legal moves for a king at a given position
std::vector<std::string> generateKingMoves(const BoardState& board, char pieceType, int position){
    std::vector<std::string> moves;
    // Implementation of king move generation logic goes here
    return moves;
}

// Function that generates all possible legal moves for a given piece at a given position
std::vector<std::string> generateMoves(const BoardState& board, char pieceType, int position) {
    std::vector<std::string> moves;
    // Implementation of move generation logic goes here
    return moves;
}

// Function that generates all possible legal moves for the current player
std::vector<std::string> generateAllMoves(const BoardState& board) {
    std::vector<std::string> allMoves;
    // Implementation of generating all moves for the current player goes here
    return allMoves;
}

