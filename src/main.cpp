// main.cpp - Simple Chess GUI using SFML
// This code sets up a basic chessboard, loads pieces from a FEN string,
// allows piece movement via mouse clicks, and outputs moves in UCI format.
// Requires SFML library. 


#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <cctype>

const int SQUARE_SIZE = 80;
const int BOARD_SIZE = 8;

std::unordered_map<char, std::string> pieceToFile = {
    {'P', "wP.png"}, {'N', "wN.png"}, {'B', "wB.png"}, {'R', "wR.png"},
    {'Q', "wQ.png"}, {'K', "wK.png"}, {'p', "bP.png"}, {'n', "bN.png"},
    {'b', "bB.png"}, {'r', "bR.png"}, {'q', "bQ.png"}, {'k', "bK.png"}
};

struct Piece {
    char type;
    sf::Sprite sprite;
    int row, col;
};

// Draw chessboard squares
void drawBoard(sf::RenderWindow& window) {
    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    sf::Color lightColor(240, 217, 181);
    sf::Color darkColor(181, 136, 99);

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            bool isLight = (row + col) % 2 == 0;
            square.setFillColor(isLight ? lightColor : darkColor);
            square.setPosition(col * SQUARE_SIZE, row * SQUARE_SIZE);
            window.draw(square);
        }
    }
}

// Load pieces from FEN
void loadPositionFromFEN(const std::string& fen, std::vector<Piece>& pieces,
                         const std::unordered_map<char, sf::Texture>& textures)
{
    pieces.clear();
    int row = 0, col = 0;

    for (char c : fen) {
        if (c == ' ') break;
        if (c == '/') {
            row++;
            col = 0;
        } else if (isdigit(c)) {
            col += c - '0';
        } else if (pieceToFile.count(c)) {
            Piece p;
            p.type = c;
            p.sprite.setTexture(textures.at(c));
            p.row = row;
            p.col = col;
            p.sprite.setPosition(col * SQUARE_SIZE, row * SQUARE_SIZE);
            pieces.push_back(p);
            col++;
        }
    }
}

// Convert pixel position to board coordinates
sf::Vector2i getSquareFromMouse(int x, int y) {
    return { y / SQUARE_SIZE, x / SQUARE_SIZE };
}

// Find piece at row/col
int findPieceAt(const std::vector<Piece>& pieces, int row, int col) {
    for (size_t i = 0; i < pieces.size(); i++)
        if (pieces[i].row == row && pieces[i].col == col)
            return i;
    return -1;
}

// Generate move string in UCI format, e.g., "e2e4"
std::string getMoveString(int fromRow, int fromCol, int toRow, int toCol) {
    char fromFile = 'a' + fromCol;
    char fromRank = '8' - fromRow;
    char toFile = 'a' + toCol;
    char toRank = '8' - toRow;

    std::string move;
    move += fromFile;
    move += fromRank;
    move += toFile;
    move += toRank;
    return move;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(SQUARE_SIZE * BOARD_SIZE, SQUARE_SIZE * BOARD_SIZE), "Chess GUI");

    // Load piece textures
    std::unordered_map<char, sf::Texture> textures;
    for (auto& [symbol, file] : pieceToFile) {
        sf::Texture tex;
        if (!tex.loadFromFile("assets/" + file)) {
            std::cerr << "Failed to load " << file << "\n";
            return -1;
        }
        textures[symbol] = tex;
    }

    std::vector<Piece> pieces;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    loadPositionFromFEN(fen, pieces, textures);

    Piece* selectedPiece = nullptr;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i square = getSquareFromMouse(event.mouseButton.x, event.mouseButton.y);
                int idx = findPieceAt(pieces, square.x, square.y);

                if (selectedPiece == nullptr) {
                    if (idx != -1) selectedPiece = &pieces[idx];
                } else {
                    // Save original position
                    int fromRow = selectedPiece->row;
                    int fromCol = selectedPiece->col;

                    // Move piece
                    selectedPiece->row = square.x;
                    selectedPiece->col = square.y;
                    selectedPiece->sprite.setPosition(square.y * SQUARE_SIZE, square.x * SQUARE_SIZE);

                    // Capture if necessary
                    if (idx != -1 && &pieces[idx] != selectedPiece)
                        pieces.erase(pieces.begin() + idx);

                    // Output move made in UCI format
                    std::string move = getMoveString(fromRow, fromCol, square.x, square.y);
                    std::cout << "Move made: " << move << std::endl;

                    selectedPiece = nullptr;
                }
            }
        }

        window.clear();
        drawBoard(window);
        for (auto& p : pieces)
            window.draw(p.sprite);
        window.display();
    }

    return 0;
}
