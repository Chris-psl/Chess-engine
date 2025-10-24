// Update the board state based on the given move

#pragma once
#include "utils.h"
#include "movegen.h"


/**
 * Updates the en passant square in the board state based on the last move.
 * If the last move was a two-square pawn advance, sets the en passant square.
 * Otherwise, resets it to -1.
 * 
 * Must be called after making a move to update the board state.
 */
void updateEnPassantSquare(BoardState& board, const Move& move);

/**
 * Updates the castling rights in the board state based on the last move.
 * Removes castling rights if the king or rook has moved.
 */
void updateCastlingRights(BoardState& board, const Move& move);

/**
 * Function that updates the castling rights and en passant square after a move
 */
void updateGameState(BoardState& board, const Move& move);

/**
 * Applies a move to the board state, updating piece bitboards and game state.
 * Handles captures, promotions, en passant, and castling rights.
 */
void applyMove(BoardState& board, const Move& move);


