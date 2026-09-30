#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

constexpr int BOARD_SIZE = 19;
constexpr int BOARD_CELLS = BOARD_SIZE * BOARD_SIZE;

// Aussi utilisé pour désigner un joueur (Black ou White).
enum class Cell : uint8_t { Empty, Black, White };

enum class Mode { PvAI, PvP };

struct Pos {
    int x;
    int y;
};

struct MoveResult {
    bool             isLegal;
    std::string      rejectionReason;  // vide si le coup est légal
    std::vector<Pos> captured;
};

struct MoveSuggestion {
    Pos              pos;
    double           elapsedMs;
    int              depth;
    long             nodesEvaluated;
    std::vector<Pos> principalVariation;
};

struct GameView {
    std::array<Cell, BOARD_CELLS> board;  // index = y * BOARD_SIZE + x
    Cell                          currentPlayer;
    int                           blackCaptureCount;
    int                           whiteCaptureCount;
    std::optional<Cell>           winner;  // vide tant que la partie continue
};
