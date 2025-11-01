// main.cpp - Chess GUI using SFML, integrated with engine applyMove(BoardState&, const Move&)
// castling test: r4kr1/8/8/8/8/8/8/R3K2R w KQ - 0 1
// castling test: p4k1p/8/8/8/8/8/8/R3K2R w KQ - 0 1
// en passant test: 8/8/8/3pP3/8/8/8/4k3 b - e3 0 1

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <cctype>
#include <optional>

// Project headers
#include "parsing.h"     // BoardState parseFEN(const std::string&);
#include "engine.h"      // std::string engine(const std::string& cmd, const std::string& fen, const BoardState& state);
#include "updateBoard.h" // void applyMove(BoardState& board, const Move& move);
#include "utils.h"       // may contain helpers / types the engine uses

const int SQUARE_SIZE = 80;
const int BOARD_SIZE = 8;

static const std::unordered_map<char, std::string> pieceToFile = {
    {'P', "wP.png"}, {'N', "wN.png"}, {'B', "wB.png"}, {'R', "wR.png"},
    {'Q', "wQ.png"}, {'K', "wK.png"}, {'p', "bP.png"}, {'n', "bN.png"},
    {'b', "bB.png"}, {'r', "bR.png"}, {'q', "bQ.png"}, {'k', "bK.png"}
};

struct Piece {
    char type;
    sf::Sprite sprite;
    int row; // 0..7 top->bottom (0 = rank 8)
    int col; // 0..7 left->right (0 = file a)
};

// --------------------------------------------------
// Helpers
// --------------------------------------------------

void drawBoard(sf::RenderWindow& window) {
    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    sf::Color light(240, 217, 181), dark(181, 136, 99);
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            bool isLight = (r + c) % 2 == 0;
            square.setFillColor(isLight ? light : dark);
            square.setPosition(static_cast<float>(c * SQUARE_SIZE), static_cast<float>(r * SQUARE_SIZE));
            window.draw(square);
        }
    }
}

void loadPositionFromFEN(const std::string& fen, std::vector<Piece>& pieces, const std::unordered_map<char, sf::Texture>& textures) {
    pieces.clear();
    int row = 0;
    int col = 0;
    for (char ch : fen) {
        if (ch == ' ') break;
        if (ch == '/') {
            ++row;
            col = 0;
        } else if (std::isdigit(static_cast<unsigned char>(ch))) {
            col += ch - '0';
        } else {
            auto it = textures.find(ch);
            if (it != textures.end()) {
                Piece p;
                p.type = ch;
                p.row = row;
                p.col = col;
                p.sprite.setTexture(it->second);
                p.sprite.setPosition(static_cast<float>(col * SQUARE_SIZE), static_cast<float>(row * SQUARE_SIZE));
                pieces.push_back(p);
            }
            ++col;
        }
    }
}

// Convert pixel mouse to board square (row, col)
sf::Vector2i getSquareFromMouse(int x, int y) {
    int col = x / SQUARE_SIZE;
    int row = y / SQUARE_SIZE;
    if (col < 0) col = 0; if (col >= BOARD_SIZE) col = BOARD_SIZE - 1;
    if (row < 0) row = 0; if (row >= BOARD_SIZE) row = BOARD_SIZE - 1;
    return { row, col };
}

int findPieceAt(const std::vector<Piece>& pieces, int row, int col) {
    for (size_t i = 0; i < pieces.size(); ++i) {
        if (pieces[i].row == row && pieces[i].col == col) return static_cast<int>(i);
    }
    return -1;
}

std::string getMoveString(int fromRow, int fromCol, int toRow, int toCol, char promotion = '\0') {
    char fromFile = 'a' + fromCol;
    char fromRank = '8' - fromRow;
    char toFile = 'a' + toCol;
    char toRank = '8' - toRow;
    std::string s;
    s += fromFile; s += fromRank; s += toFile; s += toRank;
    if (promotion != '\0') s += std::tolower(static_cast<unsigned char>(promotion));
    return s;
}

std::optional<Move> uciToMove(const std::string& uci) {
    if (uci.size() < 4) return std::nullopt;
    char fFile = uci[0], fRank = uci[1], tFile = uci[2], tRank = uci[3], promotion = uci[4];
    if (fFile < 'a' || fFile > 'h' || tFile < 'a' || tFile > 'h') return std::nullopt;
    if (fRank < '1' || fRank > '8' || tRank < '1' || tRank > '8') return std::nullopt;

    int fromFile = fFile - 'a';
    int toFile   = tFile - 'a';
    int fromRank = fRank - '1'; // 0 = rank1
    int toRank   = tRank - '1';

    int fromSq = fromRank * 8 + fromFile;
    int toSq   = toRank * 8 + toFile;



    Move m{};
    m.from = fromSq;
    m.to = toSq;
    m.promotion = promotion;
    m.isCapture = false;
    m.isEnPassant = false;
    m.isCastling = false;

    if (uci.size() >= 5) {
        char prom = static_cast<char>(std::toupper(static_cast<unsigned char>(uci[4])));
        if (prom == 'Q' || prom == 'R' || prom == 'B' || prom == 'N') m.promotion = prom;
    }
    return m;
}

sf::Vector2i squareIndexToRowCol(int sq) {
    int rankIndex = sq / 8; // 0..7 rank1 = 0
    int file = sq % 8;
    int row = 7 - rankIndex; // rank8 -> row0
    int col = file;
    return { row, col };
}

int rowColToSquareIndex(int row, int col) {
    int file = col;
    int rankIndex = 7 - row; // row0 -> rank8
    return rankIndex * 8 + file;
}

// --------------------------------------------------
// Convert BoardState -> FEN string for engine
// --------------------------------------------------
std::string bitboardsToFEN(const BoardState& board); // implement in parsing.cpp or utils.cpp

// --------------------------------------------------
// Main
// --------------------------------------------------
int main() {
    std::string mode;
    std::cout << "Enter mode (1: Engine Test, 2: GUI): ";
    std::getline(std::cin, mode);

    // Get initial FEN and setup board
    std::cout << "Enter initial FEN (or leave empty for standard start): ";
    std::string fenInput;
    std::getline(std::cin, fenInput);
    if (fenInput.empty()) fenInput = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string initialFen = fenInput;
    BoardState boardState = parseFEN(initialFen);

    // Mode 1: Engine Test
    if (mode == "1") {
        std::string r = engine("1", initialFen, boardState);
        std::cout << "Engine returned: " << r << std::endl;
        return 0;
    }

    // Mode 2: GUI
    int playerChoice = 1;
    std::cout << "Play as (1=White, 2=Black). Default 1: ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) {
        try { playerChoice = std::stoi(input); } catch (...) { playerChoice = 1; }
        if (playerChoice != 1 && playerChoice != 2) playerChoice = 1;
    }

    // Initialize SFML window
    sf::RenderWindow window(sf::VideoMode(SQUARE_SIZE * BOARD_SIZE, SQUARE_SIZE * BOARD_SIZE), "Chess GUI");

    // Load piece textures
    std::unordered_map<char, sf::Texture> textures;
    for (const auto& kv : pieceToFile) {
        char sym = kv.first;
        const std::string& fname = kv.second;
        textures.emplace(sym, sf::Texture());
        sf::Texture& tex = textures.at(sym);
        if (!tex.loadFromFile("assets/" + fname)) {
            std::cerr << "Failed to load asset: " << fname << "\n";
            return -1;
        }
    }

    // Load initial position
    std::vector<Piece> pieces;
    loadPositionFromFEN(initialFen, pieces, textures);

    // Main loop
    BoardState board = parseFEN(initialFen);
    Piece* selectedPiece = nullptr;
    int selectedIndex = -1;

    while (window.isOpen()) {
        sf::Event event;
        bool humanMovedThisFrame = false;
        std::string lastUciMove;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                break;
            }

            // --------------------------------------------------
            // Human turn
            // --------------------------------------------------
            bool humanTurn = (playerChoice == 1 && board.whiteToMove) || (playerChoice == 2 && !board.whiteToMove);

            if (humanTurn) {
                BoardState tempBoard = board;
                MoveList legalMoves = generateLegalMoves(tempBoard);

                // Check if player has lost
                if(legalMoves.moves.empty()) {
                    std::cout << "Game over! You have no legal moves.\n";
                    window.close();
                    break;
                }
            

                // Handle piece selection and movement
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i sq = getSquareFromMouse(event.mouseButton.x, event.mouseButton.y);
                    int row = sq.x; // y
                    int col = sq.y; // x
                    int idx = findPieceAt(pieces, row, col);

                    if (!selectedPiece) {
                        if (idx != -1) { selectedIndex = idx; selectedPiece = &pieces[selectedIndex]; }
                    } else {
                        int fromRow = selectedPiece->row;
                        int fromCol = selectedPiece->col;
                        int toRow = row;
                        int toCol = col;

                        if (fromRow == toRow && fromCol == toCol) { selectedPiece = nullptr; selectedIndex = -1; continue; }

                        char promotion = '\0';
                        if ((selectedPiece->type == 'P' && toRow == 0) || (selectedPiece->type == 'p' && toRow == 7)) promotion = 'Q';
                        std::string uci = getMoveString(fromRow, fromCol, toRow, toCol, promotion);

                        auto maybeMove = uciToMove(uci);
                        if (!maybeMove) { std::cerr << "Invalid UCI: " << uci << "\n"; selectedPiece = nullptr; selectedIndex = -1; continue; }
                        Move mv = *maybeMove;

                        int captureIdx = findPieceAt(pieces, toRow, toCol);
                        if (captureIdx != -1 && captureIdx != selectedIndex) mv.isCapture = true;

                        // Ensure move is legal
                        bool isLegal = false;
                        for (const auto& legalMove : legalMoves.moves) {
                            if (legalMove.from == mv.from && legalMove.to == mv.to &&
                                legalMove.promotion == mv.promotion) {
                                isLegal = true;
                                break;
                            }
                        }

                        if (!isLegal) {
                            std::cerr << "Illegal move attempted: " << uci << "\n";
                            selectedPiece = nullptr;
                            selectedIndex = -1;
                            continue;
                        }

                        // Apply move to board state
                        applyMove(board, mv);

                        if (captureIdx != -1 && captureIdx != selectedIndex) {
                            if (captureIdx < selectedIndex) { pieces.erase(pieces.begin() + captureIdx); selectedIndex--; }
                            else pieces.erase(pieces.begin() + captureIdx);
                        }

                        pieces[selectedIndex].row = toRow;
                        pieces[selectedIndex].col = toCol;
                        pieces[selectedIndex].sprite.setPosition(static_cast<float>(toCol * SQUARE_SIZE), static_cast<float>(toRow * SQUARE_SIZE));

                        lastUciMove = uci;
                        humanMovedThisFrame = true;
                        selectedPiece = nullptr;
                        selectedIndex = -1;
                    }
                }
            }
        }

        if (humanMovedThisFrame) std::cout << "Human Move: " << lastUciMove << "\n";

        // --------------------------------------------------
        // Engine turn
        // --------------------------------------------------
        bool engineTurn = !((playerChoice == 1 && board.whiteToMove) || (playerChoice == 2 && !board.whiteToMove));
        if (engineTurn) {
            std::string fenNow = bitboardsToFEN(board);
            std::string engineMoveUCI = engine("2", fenNow, board);

            if (!engineMoveUCI.empty() && engineMoveUCI.size() >= 4 && engineMoveUCI != "ff" && engineMoveUCI != "invalid command" && engineMoveUCI != "error") {
                auto maybeMove = uciToMove(engineMoveUCI);
                if (!maybeMove) { std::cerr << "Engine invalid UCI: " << engineMoveUCI << "\n"; }
                else {
                    Move mv = *maybeMove;
                    sf::Vector2i dst = squareIndexToRowCol(mv.to);
                    int captureIdx = findPieceAt(pieces, dst.x, dst.y);
                    if (captureIdx != -1) mv.isCapture = true;
                    applyMove(board, mv);

                    sf::Vector2i fromRC = squareIndexToRowCol(mv.from);
                    int idx = findPieceAt(pieces, fromRC.x, fromRC.y);
                    if (idx == -1) std::cerr << "Engine move from empty square: " << engineMoveUCI << "\n";
                    else {
                        if (captureIdx != -1 && captureIdx != idx) {
                            if (captureIdx < idx) { pieces.erase(pieces.begin() + captureIdx); idx--; }
                            else pieces.erase(pieces.begin() + captureIdx);
                        }
                        sf::Vector2i toRC = squareIndexToRowCol(mv.to);
                        pieces[idx].row = toRC.x;
                        pieces[idx].col = toRC.y;
                        pieces[idx].sprite.setPosition(static_cast<float>(toRC.y * SQUARE_SIZE), static_cast<float>(toRC.x * SQUARE_SIZE));
                    }
                }
            } else std::cerr << "Engine returned no valid move: " << engineMoveUCI << "\n";
        }

        window.clear();
        drawBoard(window);
        for (const auto& p : pieces) window.draw(p.sprite);
        window.display();

        sf::sleep(sf::milliseconds(10));
    }

    return 0;
}
