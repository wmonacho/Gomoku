#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

constexpr int BOARD_SIZE = 19;
constexpr int BOARD_CELLS = BOARD_SIZE * BOARD_SIZE;

// Aussi utilisé pour désigner un joueur (Black ou White).
// enum class Cell : uint8_t { Empty, Black, White };
enum class Cell : uint8_t {
    Empty = 0,
    Black = 1,
    White = 2
};

enum class Mode { PvAI, PvP, AIvAI };

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

struct PlayerInfo {
    std::string name;
    bool isAI;
    int capturedStones;
    float lastMoveTime;
    std::string lastMoveCoord = "-";
    Cell color;
};

struct GameView {
    std::array<Cell, BOARD_CELLS> board{};  // index = y * BOARD_SIZE + x
    // Cell                          currentPlayer = Cell::Black;
    PlayerInfo*                   currentPlayer = nullptr;
    std::optional<Cell>           winner;  // vide tant que la partie continue
    bool                          gameStarted = false;
    bool                          tipsEnable = false;
};

struct GameInfo {
    GameView gameView;
    PlayerInfo p1Info{"Player 1 (Black)", false, 0, 0.0f, "-", Cell::Black};
    PlayerInfo p2Info{"Player 2 (White)", false, 0, 0.0f, "-", Cell::White};

    GameInfo() {
        gameView.currentPlayer = &p1Info;
    }
};