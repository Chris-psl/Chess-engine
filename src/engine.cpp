// here will be implemented the hub for the engine functionalities
// engine.cpp - Chess engine core functionalities
// 8/8/4N3/3K4/8/8/8/8 w - - 0 1
// 8/pppppppp/8/4Q3/8/8/PPPPPPPP/8 w KQkq - 0 1
// castling rights update test:  r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1
// en passant test: 8/8/8/3pP3/8/8/8/8 w - d6 0 1
// 8/8/3p4/4P3/8/8/8/8 w - d3 0 1


//#include "engine.h"
#include "movegen.h"
#include "utils.h"
#include "parsing.h"
#include <iostream>
#include "tools.h"


// ============================================================================
//  SECTION 1: Main loop
// ============================================================================

int main() {

    BoardState board = {}; // Initialize an empty board state
    std::string command;
    std::cout << "Enter command: ";

    std::getline(std::cin, command);
    
    if(command == "1"){
        // Example usage: parse FEN and generate moves
        board = parseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // Another position
        //board = parseFEN("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"); // Another position
        
        // Initialize attack tables and generate moves
        initAttackTables();
        MoveList moves = generateMoves(board);

        // Print generated moves
        std::cout << "Generated " << moves.moves.size() << " moves.\n";
        while(moves.moves.size() > 0){
            Move m = moves.moves.back();
            moves.moves.pop_back();
            std::cout << squareToString(m.from) << squareToString(m.to);
            if(m.promotion != '\0')
                std::cout << m.promotion;
            std::cout << "\n";
        }
    }
    else if(command == "2"){
        // Example usage: parse FEN from user input and generate moves
        std::cout << "Enter FEN string: ";
        std::string fenInput;
        std::getline(std::cin, fenInput);
        board = parseFEN(fenInput);

        // Print the board using all the bitboards
        std::cout << "White Pawns:\n";
        printBitboard(board.whitePawns);
        std::cout << "Black Pawns:\n";
        printBitboard(board.blackPawns);

        // Initialize attack tables and generate moves
        initAttackTables();
        MoveList moves = generateMoves(board);

        // print the captures
        bool anyCapture = false;
        for (const auto& m : moves.moves) {
            if (m.isCapture) {
                anyCapture = true;
                std::cout << "Capture move: " << squareToString(m.from) << squareToString(m.to);
                if(m.promotion != '\0')
                    std::cout << m.promotion;
                std::cout << "\n";
            }
        }

        // print the en passant moves
        for (const auto& m : moves.moves) {
            if (m.isEnPassant) {
                std::cout << "En Passant move: " << squareToString(m.from) << squareToString(m.to) << "\n";
            }
        }

        // print the promotion moves
        for (const auto& m : moves.moves) {
            if (m.promotion != '\0') {
                std::cout << "Promotion move: " << squareToString(m.from) << squareToString(m.to) << " to " << m.promotion << "\n";
            }
        }   

        // print the castling moves
        for (const auto& m : moves.moves) {
            if (m.isCastling) {
                std::cout << "Castling move: " << squareToString(m.from) << squareToString(m.to) << "\n";
            }
        }

        std::cout << "Generated " << moves.moves.size()<< " moves, any capture: " << anyCapture << ".\n";
        while(moves.moves.size() > 0){
            Move m = moves.moves.back();
            moves.moves.pop_back();
            std::cout << squareToString(m.from) << squareToString(m.to);
            if(m.promotion != '\0')
                std::cout << m.promotion;
            std::cout << "\n";
        }


    }else if (command == "3"){
        // Example usage: simple engine loop
        bool white = true;
        if(white == true)board = parseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // Starting position
        else board = parseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"); // Starting position

        // Initialize attack tables and generate moves
        initAttackTables();
        MoveList moves = generateMoves(board);
        bool playing = true;

        // Game loop
        while (moves.moves.size() > 0) {
            // Apply the move

            
            // Evaluate current position
            //int score = evaluateBoard(board);

            // Generate moves for current position
            MoveList moves = generateMoves(board);

            // New round
            std::cout << "Score calculated, continue;\n";
            std::cin >> playing;
            if (!playing){
                std::cout << "Game loop broken.\n";
                break;
            }
        }
    }
}

