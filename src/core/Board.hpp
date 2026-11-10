#pragma once

#include "app/Dto.hpp"
#include <array>
#include <cassert>
#include <cstdint>

// Les 4 lignes qui passent par une case. Le sens opposé s'obtient avec {-dx, -dy}.
constexpr std::array<Pos, 4> DIRECTIONS = {{
    {1, 0},   // horizontale →
    {0, 1},   // verticale ↓
    {1, 1},   // diagonale ↘
    {1, -1},  // anti-diagonale ↗
}};

constexpr int MAX_CAPTURES_PER_MOVE = 16;
constexpr int MAX_HISTORY = 512;
// Une case vide est candidate pour l'IA si une pierre est à CANDIDATE_RADIUS cases ou moins (carré 5×5).
constexpr int CANDIDATE_RADIUS = 2;
// La partie s'arrête à 10, mais le coup gagnant peut capturer plusieurs paires d'un coup.
constexpr int MAX_CAPTURE_COUNT = 32;

// Position atteinte en avançant de n pas dans la direction dir.
inline Pos step(Pos p, Pos dir, int n = 1) {
    return {p.x + dir.x * n, p.y + dir.y * n};
}

inline Cell opponent(Cell color) {
    assert(color != Cell::Empty && "opponent() n'a pas de sens pour une case vide");
    return color == Cell::Black ? Cell::White : Cell::Black;
}

// Rangées, colonnes et les deux diagonales : 19 + 19 + 37 + 37.
constexpr int LINE_COUNT = 2 * BOARD_SIZE + 2 * (2 * BOARD_SIZE - 1);

// Poids d'une suite de pierres alignées, partagé par Board (score incrémental) et Eval (recalcul complet).
inline int runWeight(int length) {
    if (length >= 5)
        return 10'000;
    if (length == 4)
        return 1'000;
    if (length == 3)
        return 100;
    if (length == 2)
        return 10;
    return 0;
}

class Board {
public:
    Board();

    Cell     at(Pos pos) const;
    void     place(Pos pos, Cell color);
    void     capture(Pos pos);
    void     undo();
    bool     isInside(Pos pos) const;
    int      captureCount(Cell color) const;
    uint64_t hash() const;
    // Recalcul complet, lent : sert à vérifier le hash incrémental dans les tests.
    uint64_t computeHashFromScratch() const;
    // Pierres (des deux couleurs) dans le carré de rayon CANDIDATE_RADIUS autour de pos, tenu à jour à chaque coup.
    int      stonesNearby(Pos pos) const;
    // Recalcul complet, lent : sert à vérifier stonesNearby dans les tests.
    int      countStonesNearbyFromScratch(Pos pos) const;
    // Somme des suites de color sur tout le plateau (poids runWeight), tenue à jour à chaque coup.
    int      runScore(Cell color) const;
    // Paires adverses que color peut prendre au prochain coup (X O O _ ou _ O O X), tenu à jour à chaque coup.
    int      captureThreats(Cell color) const;

private:
    // Ce que vaut une ligne pour chaque couleur (index : couleur - 1).
    struct LineValue {
        std::array<int, 2> runs{};
        std::array<int, 2> threats{};
    };

    struct MoveRecord {
        Pos                                     placed;
        Cell                                    color;
        std::array<Pos, MAX_CAPTURES_PER_MOVE>  captured;
        int                                     capturedCount;
    };

    void putStone(Pos pos, Cell color);
    void clearStone(Pos pos);
    void addCaptures(Cell color, int delta);
    void addNearby(Pos pos, int delta);
    void rescoreLines(Pos pos);
    LineValue scoreLine(Pos pos, Pos dir) const;

    std::array<Cell, BOARD_CELLS>          cells_;
    std::array<MoveRecord, MAX_HISTORY>    history_;
    int                                    historySize_ = 0;
    std::array<int, 2>                     captureCounts_ = {0, 0};
    uint64_t                               hash_ = 0;
    std::array<uint8_t, BOARD_CELLS>       nearbyStones_{};
    std::array<LineValue, LINE_COUNT>      lineValues_{};
    std::array<int, 2>                     runScores_ = {0, 0};
    std::array<int, 2>                     captureThreats_ = {0, 0};
};
