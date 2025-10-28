// main.cpp - Chess GUI using SFML, integrated with engine applyMove(BoardState&, const Move&)
// Requires: SFML and your project's headers: parsing.h, engine.h, updateBoard.h (applyMove), utils.h

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <cctype>
#include <optional>

// Project headers (adapt include paths as needed)
#include "parsing.h"     // BoardState parseFEN(const std::string&);
#include "engine.h"      // std::string engine(const std::string& cmd, const std::string& fen, const BoardState& state);
#include "updateBoard.h" // void applyMove(BoardState& board, const Move& move);
#include "utils.h"       // may contain helpers / types the engine uses

const int SQUARE_SIZE = 80;
const int BOARD_SIZE = 8;

// Map piece symbols to image file names (place PNGs into assets/)
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

// Draw board
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

// load pieces from FEN into pieces vector using textures map
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

// Convert pixel mouse to board square (row, col). row 0 = top (rank 8)
sf::Vector2i getSquareFromMouse(int x, int y) {
    int col = x / SQUARE_SIZE;
    int row = y / SQUARE_SIZE;
    if (col < 0) col = 0; if (col >= BOARD_SIZE) col = BOARD_SIZE - 1;
    if (row < 0) row = 0; if (row >= BOARD_SIZE) row = BOARD_SIZE - 1;
    return { row, col };
}

// find index of piece at row/col
int findPieceAt(const std::vector<Piece>& pieces, int row, int col) {
    for (size_t i = 0; i < pieces.size(); ++i) {
        if (pieces[i].row == row && pieces[i].col == col) return static_cast<int>(i);
    }
    return -1;
}

// Build UCI string from rows/cols (row 0 = rank 8)
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

// Convert UCI like "e2e4" or "e7e8q" -> Move (engine's Move struct)
// square indexing: 0 = a1, 63 = h8
std::optional<Move> uciToMove(const std::string& uci) {
    if (uci.size() < 4) return std::nullopt;
    char fFile = uci[0];
    char fRank = uci[1];
    char tFile = uci[2];
    char tRank = uci[3];

    if (fFile < 'a' || fFile > 'h' || tFile < 'a' || tFile > 'h') return std::nullopt;
    if (fRank < '1' || fRank > '8' || tRank < '1' || tRank > '8') return std::nullopt;

    int fromFile = fFile - 'a';
    int fromRank = fRank - '1'; // 0..7 where 0 = rank1
    int toFile = tFile - 'a';
    int toRank = tRank - '1';

    int fromSq = fromRank * 8 + fromFile; // a1=0
    int toSq   = toRank * 8 + toFile;

    Move m{};
    m.from = fromSq;
    m.to = toSq;
    m.promotion = '\0';
    m.isCapture = false;
    m.isEnPassant = false;
    m.isCastling = false;

    if (uci.size() >= 5) {
        char prom = uci[4];
        prom = static_cast<char>(std::toupper(static_cast<unsigned char>(prom)));
        if (prom == 'Q' || prom == 'R' || prom == 'B' || prom == 'N') m.promotion = prom;
    }

    return m;
}

// Convert engine square (0..63) -> row,col for rendering (row 0 = top = rank8)
sf::Vector2i squareIndexToRowCol(int sq) {
    int rankIndex = sq / 8; // 0..7 where 0 = rank1
    int file = sq % 8;
    int row = 7 - rankIndex; // turn rankIndex so rank1 -> row7, rank8 -> row0
    int col = file;
    return { row, col };
}

// Convert row,col (rendering) -> engine square index (0..63)
int rowColToSquareIndex(int row, int col) {
    int file = col;
    int rankIndex = 7 - row; // row0 -> rankIndex7 (rank8)
    return rankIndex * 8 + file;
}

// --------------------------------------------------
// Main
// --------------------------------------------------
int main() {
    // Ask mode
    std::string mode;
    std::cout << "Enter mode (1: Engine Test, 2: GUI): ";
    std::getline(std::cin, mode);

    std::string initialFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    BoardState boardState = parseFEN(initialFen); // must exist in parsing.h

    if (mode == "1") {
        // engine test: call engine and print result
        std::string r = engine("1", initialFen, boardState);
        std::cout << "Engine returned: " << r << std::endl;
        return 0;
    }

    // GUI mode
    int playerChoice = 1;
    std::cout << "Play as (1=White, 2=Black). Default 1: ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) {
        try { playerChoice = std::stoi(input); } catch (...) { playerChoice = 1; }
        if (playerChoice != 1 && playerChoice != 2) playerChoice = 1;
    }

    // Create window
    sf::RenderWindow window(sf::VideoMode(SQUARE_SIZE * BOARD_SIZE, SQUARE_SIZE * BOARD_SIZE), "Chess GUI");

    // Load textures into map (persistent)
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

    // Pieces vector initialised from FEN
    std::vector<Piece> pieces;
    loadPositionFromFEN(initialFen, pieces, textures);

    // BoardState used by engine and move applier
    BoardState board = parseFEN(initialFen);

    Piece* selectedPiece = nullptr;
    int selectedIndex = -1;

    // main loop
    while (window.isOpen()) {
        sf::Event event;
        bool humanMovedThisFrame = false;
        std::string lastUciMove;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                break;
            }

            // Only react to mouse if it's human's turn
            bool humanTurn = (playerChoice == 1 && board.whiteToMove) || (playerChoice == 2 && !board.whiteToMove);

            if (humanTurn && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i sq = getSquareFromMouse(event.mouseButton.x, event.mouseButton.y);
                int row = sq.x;
                int col = sq.y;
                int idx = findPieceAt(pieces, row, col);

                if (selectedPiece == nullptr) {
                    // pick up piece
                    if (idx != -1) {
                        selectedIndex = idx;
                        selectedPiece = &pieces[selectedIndex];
                    }
                } else {
                    // Try to move selected piece to clicked square
                    int fromRow = selectedPiece->row;
                    int fromCol = selectedPiece->col;
                    int toRow = row;
                    int toCol = col;

                    // Deselect if same square clicked
                    if (fromRow == toRow && fromCol == toCol) {
                        selectedPiece = nullptr;
                        selectedIndex = -1;
                        continue;
                    }

                    // Build UCI string (no promotion chosen here)
                    std::string uci = getMoveString(fromRow, fromCol, toRow, toCol);

                    // Convert UCI to Move
                    auto maybeMove = uciToMove(uci);
                    if (!maybeMove) {
                        std::cerr << "Invalid UCI created: " << uci << "\n";
                        selectedPiece = nullptr;
                        selectedIndex = -1;
                        continue;
                    }
                    Move mv = *maybeMove;

                    // mark capture (if piece exists on dest)
                    int captureIdx = findPieceAt(pieces, toRow, toCol);
                    if (captureIdx != -1 && captureIdx != selectedIndex) mv.isCapture = true;

                    // Apply move to BoardState via applyMove
                    applyMove(board, mv); // expects to update board (including toggling side-to-move)

                    // Apply move visually on pieces vector
                    // If capture exists and captureIdx != selectedIndex, erase it safely (adjust selectedIndex if needed)
                    if (captureIdx != -1 && captureIdx != selectedIndex) {
                        if (captureIdx < selectedIndex) {
                            pieces.erase(pieces.begin() + captureIdx);
                            selectedIndex--; // shift
                            selectedPiece = &pieces[selectedIndex];
                        } else {
                            pieces.erase(pieces.begin() + captureIdx);
                        }
                    }

                    // Move selected piece
                    pieces[selectedIndex].row = toRow;
                    pieces[selectedIndex].col = toCol;
                    pieces[selectedIndex].sprite.setPosition(static_cast<float>(toCol * SQUARE_SIZE), static_cast<float>(toRow * SQUARE_SIZE));

                    lastUciMove = uci;
                    humanMovedThisFrame = true;

                    // Deselect
                    selectedPiece = nullptr;
                    selectedIndex = -1;
                }
            }
        } // event loop end

        // If human moved, print and continue (engine will play next loop)
        if (humanMovedThisFrame) {
            std::cout << "Human Move: " << lastUciMove << "\n";
        }

        // If it's engine's turn, request a move and apply it once
        bool engineTurn = !((playerChoice == 1 && board.whiteToMove) || (playerChoice == 2 && !board.whiteToMove));
        if (engineTurn) {
            // Call engine - assumed signature: engine(cmd, fenString, BoardState)
            // If engine uses the BoardState param to decide, we pass current board.
            std::string engineMoveUCI = engine("2", initialFen, board);

            // If engine returned a 4+ length move
            if (!engineMoveUCI.empty() && engineMoveUCI.size() >= 4 && engineMoveUCI != "ff" && engineMoveUCI != "invalid command" && engineMoveUCI != "error") {
                auto maybeMove = uciToMove(engineMoveUCI);
                if (!maybeMove) {
                    std::cerr << "Engine returned invalid UCI: " << engineMoveUCI << "\n";
                } else {
                    Move mv = *maybeMove;

                    // Mark capture if a piece is present at destination
                    sf::Vector2i dst = squareIndexToRowCol(mv.to);
                    int captureIdx = findPieceAt(pieces, dst.x, dst.y);

                    if (captureIdx != -1) mv.isCapture = true;

                    // Apply to BoardState
                    applyMove(board, mv);

                    // Update pieces vector: find piece at from-square
                    sf::Vector2i fromRC = squareIndexToRowCol(mv.from);
                    int idx = findPieceAt(pieces, fromRC.x, fromRC.y);
                    if (idx == -1) {
                        std::cerr << "Engine attempted to move piece from empty square: " << engineMoveUCI << "\n";
                    } else {
                        // Handle capture erasure safely
                        if (captureIdx != -1 && captureIdx != idx) {
                            if (captureIdx < idx) {
                                pieces.erase(pieces.begin() + captureIdx);
                                idx--; // shift
                            } else {
                                pieces.erase(pieces.begin() + captureIdx);
                            }
                        }

                        // Move piece
                        sf::Vector2i toRC = squareIndexToRowCol(mv.to);
                        pieces[idx].row = toRC.x;
                        pieces[idx].col = toRC.y;
                        pieces[idx].sprite.setPosition(static_cast<float>(toRC.y * SQUARE_SIZE), static_cast<float>(toRC.x * SQUARE_SIZE));
                        std::cout << "Engine Move: " << engineMoveUCI << "\n";
                    }
                }
            } else {
                std::cerr << "Engine returned no valid move: " << engineMoveUCI << "\n";
            }
        }

        // Render
        window.clear();
        drawBoard(window);
        for (const auto& p : pieces) window.draw(p.sprite);
        window.display();

        // small sleep to avoid busy-loop
        sf::sleep(sf::milliseconds(10));
    } // window loop

    return 0;
}
