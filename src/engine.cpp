// here will be implemented the hub for the engine functionalities
// engine.cpp - Chess engine core functionalities
// knight test: 8/8/4N3/3K4/8/8/8/8 w - - 0 1
// queen test: 8/pppppppp/8/4Q3/8/8/PPPPPPPP/8 w KQkq - 0 1
// castling rights update test:  r3k2r/8/8/8/8/8/8/R3Kp1R w KQkq - 0 1
// castling move test:  r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1
// en passant test: 8/8/8/3pP3/8/8/8/8 w - d6 0 1
// capture test: 8/8/3p4/4P3/8/8/8/8 w - d3 0 1
// Ban king exposure test: 8/8/8/8/3n4/8/4K3/8 w - - 0 1
// ban illegal board state test: 8/8/8/8/8/8/3k4/4K3 w - - 0 1

#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <future>

#include "engine.h"
#include "movegen.h"
#include "utils.h"
#include "parsing.h"
#include "updateBoard.h"
#include "tools.h"
#include "search.h"
#include "threadPool.h"
#include "zobrist.h"
#include "transposition.h"

TranspositionTable TT(64); // 64 MB global TT

// ============================================================================
//  SECTION 1: Main loop
// ============================================================================

/**
 * @brief Main engine loop to process commands and perform actions.
 * receives a fen input and generates moves or runs the engine to find the best move based on the command given.
 * returns the best move found.
 */
std::string engine(std::string command, std::string fenInput, BoardState& board) {

    //BoardState board = {}; // Initialize an empty board state
    
    if(command == "1"){
        //////////////////////// Functionality test ////////////////////////

        // Must always initialize the attack tables after each update, mandatory for functions like is legalmovestate
        initAttackTables();

        // Check if the state is legal
        if(!isLegalMoveState(board)){
            std::cout << "Illegal board state detected!\n";
            return "error";
        }else{
            std::cout << "Legal board state.\n";
        }

        // Print parsed board state (for verification)
        std::cout << "Parsed Board State:\n";
        printBoard(board);

        // Initialize attack tables and generate moves
        //initAttackTables(); we initialized them above
        MoveList moves = generateLegalMoves(board);
        MoveList moves2 = moves;
        Move testMove = moves.moves[1]; // Take the first move for testing

        // Print all generated moves
        std::cout << "Generated " << moves.moves.size() << ".\n";
        while((moves.moves.size() > 0)){
            Move m = moves.moves.back();
            moves.moves.pop_back();
            std::cout << squareToString(m.from) << squareToString(m.to);
            if(m.promotion != '\0')
                std::cout << m.promotion;
            if(m.isCapture)
                std::cout << "x";
            if(m.isCastling)
                std::cout << "c";
            if(m.isEnPassant)
                std::cout << "ep";
            std::cout << "\n";
        }
        // Apply a castling move
        while(moves2.moves.size() > 0){
            BoardState temp = board;
            Move m = moves2.moves.back();
            moves2.moves.pop_back();
            if(m.isCastling){
                std::cout << "entered if";
                applyMove(temp, m);
                printBoard(temp);
            }
            
        }

        // Test the updateBoard function, copy the board state update the move and print the new board
        if(testMove.from != testMove.to){
            std::cout << "Applying move: " << squareToString(testMove.from) << squareToString(testMove.to) << "\n";
            applyMove(board, testMove);
            std::cout << "Board after move:\n";
            printBoard(board);
        }
        return "finished tests";

    }else if (command == "2"){
        //////////////////////// Main implementation ////////////////////////

        // Initiate zobrist hashing
        initZobrist();

        // Initialize attack tables and generate moves
        initAttackTables();
        MoveList moves = generateLegalMoves(board);

        //////////////////////// Min-max with thread Pool Implementation ////////////////////////
        size_t numThreads = 11;
        ThreadPool pool(numThreads);

        std::vector<std::future<std::pair<Move, int>>> futures;

        // Enqueue tasks in the pool
        for (const auto& m : moves.moves) {
            futures.push_back(pool.enqueue([board, m]() -> std::pair<Move,int> {
                BoardState newBoard = board;
                applyMove(newBoard, m);

                if (!isLegalMoveState(newBoard)) {
                    return {m, std::numeric_limits<int>::min()};
                }

                // int eval = minimax(newBoard, 3, false);
                int eval = minimax(newBoard, 3, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false);
                return {m, eval};
            }));
        }

        // Collect results and find the best move
        Move bestMove;
        int bestEval = std::numeric_limits<int>::min();
        bool foundMove = false;

        for (auto &fut : futures) {
            auto [move, eval] = fut.get();
            if (eval > bestEval) {
                bestEval = eval;
                bestMove = move;
                foundMove = true;
            }
        }

        // Print best move
        if (foundMove) {
            std::string bestMoveStr = squareToString(bestMove.from) + squareToString(bestMove.to);
            if (bestMove.promotion != '\0') // checks if there is promotion and adds it to the string.
                bestMoveStr.push_back(bestMove.promotion);

            // Print the move for debug 
            std::cout << "\nBest Move: " << bestMoveStr << " Evaluation: " << bestEval << "\n";

            return bestMoveStr;
        } else {
            // No moves available
            std::cout << "No legal moves found.\n\n";
            return "ff";
        }
    }
    return "invalid command";
}



// ============================================================================
//  Function dump
// ============================================================================


        ///////////////////////// Regular Minmax ////////////////////////

        // std::vector<std::future<std::pair<Move, int>>> futures;

        // // Launch each move in a separate thread
        // for (const auto& m : moves.moves) {
        //     futures.push_back(std::async(std::launch::async, [board, m]() -> std::pair<Move,int> {
        //         BoardState newBoard = board;
        //         applyMove(newBoard, m);

        //         if (!isLegalMoveState(newBoard)) {
        //             return {m, std::numeric_limits<int>::min()}; // Illegal moves get minimal evaluation
        //         }

        //         int eval = minimax(newBoard, 3, false); // Depth 3
        //         return {m, eval};
        //     }));
        // }

        // Create a thread pool with as many threads as hardware supports
        // size_t numThreads = std::thread::hardware_concurrency();
        // if (numThreads == 0) numThreads = 4; // Fallback default

