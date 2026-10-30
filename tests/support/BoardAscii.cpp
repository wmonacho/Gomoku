#include "support/BoardAscii.hpp"

#include <cassert>
#include <cstdio>

namespace {

    char symbolOf(Cell cell) {
        switch (cell) {
            case Cell::Black: return 'X';
            case Cell::White: return 'O';
            case Cell::Empty: return '.';
        }
        return '?';
    }
}

Board boardFrom(std::initializer_list<std::string_view> rows) {
    assert(rows.size() <= BOARD_SIZE && "dessin plus haut que le plateau");

    Board board;
    int y = 0;
    for (std::string_view row : rows) {
        assert(row.size() <= BOARD_SIZE && "ligne du dessin plus large que le plateau");
        for (int x = 0; x < static_cast<int>(row.size()); x++) {
            const char symbol = row[x];
            if (symbol == 'X')
                board.place({x, y}, Cell::Black);
            else if (symbol == 'O')
                board.place({x, y}, Cell::White);
            else
                assert((symbol == '.' || symbol == '_') && "caractère inconnu dans le dessin : X, O, . ou _");
        }
        y++;
    }
    return board;
}

std::vector<std::string> toRows(const Board& board) {
    std::vector<std::string> rows;
    for (int y = 0; y < BOARD_SIZE; y++) {
        std::string row;
        for (int x = 0; x < BOARD_SIZE; x++)
            row += symbolOf(board.at({x, y}));
        rows.push_back(row);
    }
    return rows;
}

std::string toAscii(const Board& board) {
    char buffer[8];
    std::string text = "    ";
    for (int x = 0; x < BOARD_SIZE; x++) {
        std::snprintf(buffer, sizeof(buffer), "%3d", x);
        text += buffer;
    }
    text += '\n';

    const std::vector<std::string> rows = toRows(board);
    for (int y = 0; y < BOARD_SIZE; y++) {
        std::snprintf(buffer, sizeof(buffer), "%3d ", y);
        text += buffer;
        for (char symbol : rows[y]) {
            text += "  ";
            text += symbol;
        }
        text += '\n';
    }

    std::snprintf(buffer, sizeof(buffer), "%d", board.captureCount(Cell::Black));
    text += "    Captures : X = " + std::string(buffer);
    std::snprintf(buffer, sizeof(buffer), "%d", board.captureCount(Cell::White));
    text += ", O = " + std::string(buffer) + '\n';
    return text;
}

void printBoard(const Board& board) {
    std::fputs(toAscii(board).c_str(), stdout);
}
