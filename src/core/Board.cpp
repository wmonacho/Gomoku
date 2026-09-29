#include "core/Board.hpp"

#include <cassert>

namespace {

    int index(Pos p) {
        assert(p.x >= 0 && p.x < BOARD_SIZE && p.y >= 0 && p.y < BOARD_SIZE
               && "position hors du plateau : tester isInside() avant");
        return p.y * BOARD_SIZE + p.x;
    }

    int colorIndex(Cell color) {
        assert(color != Cell::Empty && "une case vide n'a pas d'index de couleur");
        return static_cast<int>(color) - 1;
    }

    struct ZobristKeys {
        std::array<std::array<uint64_t, 2>, BOARD_CELLS>           stones;
        std::array<std::array<uint64_t, MAX_CAPTURE_COUNT + 1>, 2> captures;
        uint64_t                                                   sideToMove;
    };

    constexpr uint64_t splitmix64(uint64_t& state) {
        uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }

    // Graine fixe : mêmes clés à chaque exécution, donc bugs reproductibles.
    constexpr ZobristKeys makeZobristKeys() {
        ZobristKeys keys{};
        uint64_t state = 0x6A09E667F3BCC908ULL;
        for (auto& cell : keys.stones)
            for (auto& key : cell)
                key = splitmix64(state);
        for (auto& color : keys.captures)
            for (auto& key : color)
                key = splitmix64(state);
        keys.sideToMove = splitmix64(state);
        return keys;
    }

    constexpr ZobristKeys ZOBRIST = makeZobristKeys();
}

Board::Board() {
    cells_.fill(Cell::Empty);
    hash_ = computeHashFromScratch();
}

Cell Board::at(Pos pos) const {
    return cells_[index(pos)];
}

void Board::place(Pos pos, Cell color) {
    assert(historySize_ < MAX_HISTORY && "historique plein : trop de coups sans undo");
    assert(color != Cell::Empty && "place() doit poser une vraie pierre");
    assert(at(pos) == Cell::Empty && "place() sur une case déjà occupée");

    putStone(pos, color);
    hash_ ^= ZOBRIST.sideToMove;

    MoveRecord& record = history_[historySize_];
    record.placed = pos;
    record.color = color;
    record.capturedCount = 0;
    historySize_++;
}

void Board::capture(Pos pos) {
    assert(historySize_ > 0 && "capture() sans coup posé pour l'enregistrer");

    MoveRecord& record = history_[historySize_ - 1];
    assert(record.capturedCount < MAX_CAPTURES_PER_MOVE && "plus de 16 pierres capturées en un coup");
    assert(at(pos) == opponent(record.color) && "capture() d'une pierre qui n'est pas adverse");

    record.captured[record.capturedCount] = pos;
    record.capturedCount++;
    clearStone(pos);
    addCaptures(record.color, 1);
}

void Board::undo() {
    assert(historySize_ > 0 && "undo() sans coup à annuler");

    historySize_--;
    const MoveRecord& record = history_[historySize_];

    addCaptures(record.color, -record.capturedCount);
    const Cell capturedColor = opponent(record.color);
    for (int i = record.capturedCount - 1; i >= 0; i--)
        putStone(record.captured[i], capturedColor);

    hash_ ^= ZOBRIST.sideToMove;
    clearStone(record.placed);
}

bool Board::isInside(Pos pos) const {
    return pos.x >= 0 && pos.x < BOARD_SIZE && pos.y >= 0 && pos.y < BOARD_SIZE;
}

int Board::captureCount(Cell color) const {
    return captureCounts_[colorIndex(color)];
}

uint64_t Board::hash() const {
    return hash_;
}

uint64_t Board::computeHashFromScratch() const {
    uint64_t hash = 0;
    for (int i = 0; i < BOARD_CELLS; i++)
        if (cells_[i] != Cell::Empty)
            hash ^= ZOBRIST.stones[i][colorIndex(cells_[i])];
    for (int c = 0; c < 2; c++)
        hash ^= ZOBRIST.captures[c][captureCounts_[c]];
    if (historySize_ % 2 == 1)
        hash ^= ZOBRIST.sideToMove;
    return hash;
}

void Board::putStone(Pos pos, Cell color) {
    cells_[index(pos)] = color;
    hash_ ^= ZOBRIST.stones[index(pos)][colorIndex(color)];
}

void Board::clearStone(Pos pos) {
    hash_ ^= ZOBRIST.stones[index(pos)][colorIndex(at(pos))];
    cells_[index(pos)] = Cell::Empty;
}

void Board::addCaptures(Cell color, int delta) {
    int& count = captureCounts_[colorIndex(color)];
    hash_ ^= ZOBRIST.captures[colorIndex(color)][count];
    count += delta;
    assert(count >= 0 && count <= MAX_CAPTURE_COUNT && "compteur de captures hors limites");
    hash_ ^= ZOBRIST.captures[colorIndex(color)][count];
}
