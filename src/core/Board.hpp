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

private:
    struct MoveRecord {
        Pos                                     placed;
        Cell                                    color;
        std::array<Pos, MAX_CAPTURES_PER_MOVE>  captured;
        int                                     capturedCount;
    };

    void putStone(Pos pos, Cell color);
    void clearStone(Pos pos);
    void addCaptures(Cell color, int delta);

    std::array<Cell, BOARD_CELLS>          cells_;
    std::array<MoveRecord, MAX_HISTORY>    history_;
    int                                    historySize_ = 0;
    std::array<int, 2>                     captureCounts_ = {0, 0};
    uint64_t                               hash_ = 0;
};
