// main.cpp - Chess GUI using SFML, integrated with engine applyMove(BoardState&, const Move&)

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <cctype>
#include <optional>
#include <algorithm>

#include "parsing.h"     // BoardState parseFEN(const std::string&);
#include "engine.h"      // std::string engine(const std::string& cmd, const std::string& fen, const BoardState& state);
#include "updateBoard.h" // void applyMove(BoardState& board, const Move& move);
#include "utils.h"       // helpers/types the engine uses (Move, MoveList, BoardState, etc.)

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

// Draw chessboard
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

// FEN -> pieces (row 0 = rank8)
void loadPositionFromFEN(const std::string& fen, std::vector<Piece>& pieces, const std::unordered_map<char, sf::Texture>& textures) {
    pieces.clear();
    int row = 0;
    int col = 0;
    for (char ch : fen) {
        if (ch == ' ') break;
        if (ch == '/') { ++row; col = 0; }
        else if (std::isdigit(static_cast<unsigned char>(ch))) {
            col += (ch - '0');
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
            } else {
                std::cerr << "Warning: no texture for '" << ch << "' found in textures map.\n";
            }
            ++col;
        }
    }
}

// Convert pixel mouse to board square (row, col)
sf::Vector2i getSquareFromMouse(int x, int y) {
    int col = x / SQUARE_SIZE;
    int row = y / SQUARE_SIZE;
    if (col < 0) col = 0; 
    if (col >= BOARD_SIZE) col = BOARD_SIZE - 1;
    if (row < 0) row = 0; 
    if (row >= BOARD_SIZE) row = BOARD_SIZE - 1;
    return { row, col };
}

// Find piece index at (row, col), -1 if not found
int findPieceAt(const std::vector<Piece>& pieces, int row, int col) {
    for (size_t i = 0; i < pieces.size(); ++i) {
        if (pieces[i].row == row && pieces[i].col == col) return static_cast<int>(i);
    }
    return -1;
}

// Convert move from (fromRow, fromCol) to (toRow, toCol) with optional promotion to UCI string
std::string getMoveString(int fromRow, int fromCol, int toRow, int toCol, char promotion = '\0') {
    char fromFile = 'a' + fromCol;
    char fromRank = '8' - fromRow;
    char toFile = 'a' + toCol;
    char toRank = '8' - toRow;
    std::string s;
    s += fromFile; s += fromRank; s += toFile; s += toRank;
    if (promotion != '\0') s += static_cast<char>(std::tolower(static_cast<unsigned char>(promotion)));
    return s;
}

// Convert UCI string to Move struct
std::optional<Move> uciToMove(const std::string& uci) {
    if (uci.size() < 4) return std::nullopt;
    char fFile = uci[0], fRank = uci[1], tFile = uci[2], tRank = uci[3];
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
    m.promotion = '\0';
    m.isCapture = false;
    m.isEnPassant = false;
    m.isCastling = false;

    if (uci.size() >= 5) {
        char prom = static_cast<char>(std::toupper(static_cast<unsigned char>(uci[4])));
        if (prom == 'Q' || prom == 'R' || prom == 'B' || prom == 'N') m.promotion = prom;
    }
    return m;
}

// Convert square index to (row, col)
sf::Vector2i squareIndexToRowCol(int sq) {
    int rankIndex = sq / 8; // 0..7 rank1 = 0
    int file = sq % 8;
    int row = 7 - rankIndex; // rank8 -> row0
    int col = file;
    return { row, col };
}

// Convert (row, col) to square index
int rowColToSquareIndex(int row, int col) {
    int file = col;
    int rankIndex = 7 - row; // row0 -> rank8
    return rankIndex * 8 + file;
}

// Handle castling rook move in GUI pieces
void handleCastlingForKing(std::vector<Piece>& pieces, int kingIndex,
    
    int fromRow, int fromCol, int toRow, int toCol) {
    if (kingIndex < 0 || kingIndex >= static_cast<int>(pieces.size())) return;
    char k = pieces[kingIndex].type;
    if (std::toupper(static_cast<unsigned char>(k)) != 'K') return;

    int fileDiff = std::abs(fromCol - toCol);
    if (fileDiff != 2) return; // not a castling king move

    // Determine expected rook source and destination
    // Kingside: king to g-file (col 6) -> rook from h-file (col 7) to f-file (col 5)
    // Queenside: king to c-file (col 2) -> rook from a-file (col 0) to d-file (col 3)
    int rookFromCol = (toCol == 6) ? 7 : 0;
    int rookToCol   = (toCol == 6) ? 5 : 3;
    int rookRow = fromRow; // same row as king

    int rookIdx = findPieceAt(pieces, rookRow, rookFromCol);
    if (rookIdx == -1) {
        // fallback: find a rook of same color on the same row (closest to expected side)
        char expectedRookChar = std::isupper(static_cast<unsigned char>(k)) ? 'R' : 'r';
        int bestIdx = -1;
        int bestDist = 100;
        for (size_t i = 0; i < pieces.size(); ++i) {
            if (static_cast<int>(i) == kingIndex) continue;
            if (pieces[i].row != rookRow) continue;
            if (pieces[i].type != expectedRookChar) continue;
            int dist = std::abs(pieces[i].col - rookFromCol);
            if (dist < bestDist) { bestDist = dist; bestIdx = static_cast<int>(i); }
        }
        rookIdx = bestIdx;
        if (rookIdx == -1) {
            std::cerr << "Castling: rook not found for king at (" << fromRow << "," << fromCol << ")->(" << toRow << "," << toCol << ")\n";
            return;
        }
    }

    // If rook index equals king index something is deeply wrong; guard against it.
    if (rookIdx == kingIndex) {
        std::cerr << "Castling: rook index equals king index (aborting).\n";
        return;
    }

    // Move rook in GUI
    pieces[rookIdx].col = rookToCol;
    pieces[rookIdx].row = rookRow;
    pieces[rookIdx].sprite.setPosition(static_cast<float>(rookToCol * SQUARE_SIZE),
                                       static_cast<float>(rookRow * SQUARE_SIZE));

    std::cerr << "Castling: moved rook (piece idx " << rookIdx << ") from col " << rookFromCol << " to " << rookToCol << " on row " << rookRow << ".\n";
}


// --------------------------------------------------
// Main
// --------------------------------------------------
int main() {
    std::string mode;
    std::cout << "Enter mode (1: Engine Test, 2: GUI, 3: self-play): ";
    std::getline(std::cin, mode);

    // Get initial FEN and setup board
    std::cout << "Enter initial FEN (or leave empty for standard start): ";
    std::string fenInput;
    std::getline(std::cin, fenInput);
    if (fenInput.empty()) fenInput = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    BoardState boardState = parseFEN(fenInput);

    if (mode == "1") {
        std::string r = engine("1", fenInput, boardState);
        std::cout << "Engine returned: " << r << std::endl;
        return 0;
    }

    int playerChoice = 0;
    std::cout << "Play as (0=White, 1=Black). Default 0: ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) {
        try { playerChoice = std::stoi(input); } catch (...) { playerChoice = 0; }
        if (playerChoice != 0 && playerChoice != 1) playerChoice = 0;
    }

    // Initialize SFML window
    sf::RenderWindow window(sf::VideoMode(SQUARE_SIZE * BOARD_SIZE, SQUARE_SIZE * BOARD_SIZE), "Chess GUI");

    // Load piece textures
    std::unordered_map<char, sf::Texture> textures;
    for (const auto& kv : pieceToFile) {
        textures.emplace(kv.first, sf::Texture());
        sf::Texture& tex = textures.at(kv.first);
        if (!tex.loadFromFile("assets/" + kv.second)) {
            std::cerr << "Failed to load asset: " << kv.second << "\n";
            // continue; we allow the program to run but sprite will be blank
        }
    }

    // Load initial position
    std::vector<Piece> pieces;
    loadPositionFromFEN(fenInput, pieces, textures);

    // Main loop state
    BoardState board = parseFEN(fenInput);
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
            bool humanTurn = (playerChoice == 0 && board.whiteToMove) || (playerChoice == 1 && !board.whiteToMove);

            if (humanTurn) {
                if (mode == "3") break; // skip human input in self-play mode
                BoardState tempBoard = board;

                // Generate moves for the player to prevent illegal moves
                initAttackTables();
                MoveList legalMoves = generateLegalMoves(tempBoard);

                // Check if player has lost
                if(legalMoves.moves.empty()) {
                    std::cout << "Game over! You have no moves.\n";
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

                        // Construct UCI move
                        if (fromRow == toRow && fromCol == toCol) { selectedPiece = nullptr; selectedIndex = -1; continue; }

                        char promotion = '\0';
                        // White pawns promote on row 0 (rank 8). Black pawns on row 7 (rank 1).
                        if (selectedPiece->type == 'P' && toRow == 0) promotion = 'Q';
                        else if (selectedPiece->type == 'p' && toRow == 7) promotion = 'q';

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
                        updateEnPassantSquare(board, mv);
                        applyMove(board, mv);

                        // Change the icon if there is promotion
                        if (mv.promotion != '\0') {
                            bool whitePromoting = std::isupper(selectedPiece->type);
                            char promoChar = whitePromoting ? mv.promotion
                                                            : static_cast<char>(std::tolower(mv.promotion));
                            selectedPiece->type = promoChar;
                            auto it = textures.find(promoChar);
                            if (it != textures.end()) selectedPiece->sprite.setTexture(it->second);
                        }

                        // Handle captures
                        if (captureIdx != -1 && captureIdx != selectedIndex) {
                            if (captureIdx < selectedIndex) { pieces.erase(pieces.begin() + captureIdx); selectedIndex--; }
                            else pieces.erase(pieces.begin() + captureIdx);
                        }

                        // Move king/piece sprite
                        pieces[selectedIndex].row = toRow;
                        pieces[selectedIndex].col = toCol;
                        pieces[selectedIndex].sprite.setPosition(static_cast<float>(toCol * SQUARE_SIZE), static_cast<float>(toRow * SQUARE_SIZE));

                        // Handle castling
                        handleCastlingForKing(pieces, selectedIndex, fromRow, fromCol, toRow, toCol);

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
        bool engineTurn = false;
        if(mode == "3") engineTurn = true;
        else engineTurn = !((playerChoice == 0 && board.whiteToMove) || (playerChoice == 1 && !board.whiteToMove));
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

                    // Apply move to board state
                    updateEnPassantSquare(board, mv);
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

                        // Handle promotion
                        if (mv.promotion != '\0') {
                            bool whitePromoting = std::isupper(pieces[idx].type);
                            char promoChar = whitePromoting ? mv.promotion : static_cast<char>(std::tolower(mv.promotion));
                            pieces[idx].type = promoChar;
                            auto it = textures.find(promoChar);
                            if (it != textures.end()) pieces[idx].sprite.setTexture(it->second);
                        }

                        // Handle castling if king moved two squares
                        handleCastlingForKing(pieces, idx, fromRC.x, fromRC.y, toRC.x, toRC.y);
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
