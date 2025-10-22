// here will be implemented the hub for the engine functionalities
// engine.cpp - Chess engine core functionalities
// 8/8/8/3K4/8/8/8/8 w - - 0 1
// 8/pppppppp/8/8/8/8/PPPPPPPP/8 b KQkq - 0 1


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
        board = parseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // Another position
        //board = parseFEN("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"); // Another position
        initAttackTables();
        MoveList moves = generateMoves(board);
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
        std::cout << "Enter FEN string: ";
        std::string fenInput;
        std::getline(std::cin, fenInput);
        board = parseFEN(fenInput);
        initAttackTables();
        MoveList moves = generateMoves(board);
        bool anyCapture = false;
        for (const auto& m : moves.moves) {
            if (m.isCapture) {
                anyCapture = true;
                break;
            }
        }

        std::cout << "Generated " << moves.moves.size()<< " moves, any capture: " << (anyCapture ? "yes" : "no") << ".\n";


        while(moves.moves.size() > 0){
            Move m = moves.moves.back();
            moves.moves.pop_back();
            std::cout << squareToString(m.from) << squareToString(m.to);
            if(m.promotion != '\0')
                std::cout << m.promotion;
            std::cout << "\n";
        }
    }

    // while (true) {
    //     if (command == "quit") break;
    //     MoveList moves = generateMoves(board);
        
    // }
}