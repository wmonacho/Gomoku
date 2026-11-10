#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

constexpr int BOARD_SIZE = 19;
constexpr int BOARD_CELLS = BOARD_SIZE * BOARD_SIZE;

// Aussi utilisé pour désigner un joueur (Black ou White).
enum class Cell : uint8_t {
    Empty = 0,
    Black = 1,
    White = 2
};

enum class Mode { PvAI, PvP, AIvAI };

// Bonus : niveau de l'IA, traduit en profondeur de recherche par SearchConfig::depthOf.
enum class Difficulty { Easy, Medium, Hard };

enum class WinReason {
    Align = 'A',
    Captures = 'C' ,
    Forfeit = 'F',
    Timeout = 'T',
    Draw = 'D',  // plus aucun coup légal : pas de gagnant
};

enum class StopReason { MaxDepth, TimeUp };

struct Pos {
    int x;
    int y;

    bool operator==(const Pos&) const = default;  // compare x et y
};

// Hors plateau : playMove le refuse, contrairement à {0, 0} qui peut être jouable ou occupée.
constexpr Pos NO_MOVE = {-1, -1};

struct MoveResult {
    bool             isLegal;
    std::string      rejectionReason;  // vide si le coup est légal
    std::vector<Pos> captured;
};

// Un coup étudié à la racine, pour suivre le raisonnement de l'IA.
struct RootMove {
    Pos  pos;
    int  score;
    bool exact;  // false : l'alpha-bêta a seulement prouvé que le coup ne vaut pas plus que score
};

struct MoveSuggestion {
    Pos              pos;
    double           elapsedMs;
    int              depth;
    long             nodesEvaluated;
    std::vector<Pos> principalVariation;
    StopReason       stopReason;
    bool             forcedWin;  // l'IA gagne quoi que joue l'adversaire
    int              score;      // note du coup pour le joueur qui joue : > 0 = avantage, énorme = victoire forcée
    std::vector<RootMove> rootMoves;  // coups étudiés à la dernière profondeur terminée, du meilleur au moins bon
};

// Coup joué par l'IA : ce que la recherche a trouvé, et ce que le coup a fait sur le plateau.
struct AIMove {
    MoveSuggestion suggestion;
    MoveResult     result;
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
    PlayerInfo*                   currentPlayer = nullptr;
    std::optional<Cell>           winner;  // vide tant que la partie continue, et en cas de nulle
    std::optional<WinReason>      winReason;  // vide tant que la partie continue
    bool                          gameStarted = false;
    bool                          tipsEnable = false;
};

struct GameInfo {
    Mode       selectedMode;
    Difficulty difficulty = Difficulty::Hard;  // Hard : le minimum du sujet (profondeur 10)

    GameView gameView;

    PlayerInfo p1Info{"Player 1 (Black)", false, 0, 0.0f, "-", Cell::Black};
    PlayerInfo p2Info{"Player 2 (White)", false, 0, 0.0f, "-", Cell::White};
};
