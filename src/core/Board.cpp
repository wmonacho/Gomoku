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

    // Numéro de la ligne qui passe par pos dans la direction DIRECTIONS[d], de 0 à LINE_COUNT - 1.
    int lineIndex(Pos pos, int d) {
        switch (d) {
            case 0:  return pos.y;                                            // rangée
            case 1:  return BOARD_SIZE + pos.x;                               // colonne
            case 2:  return 2 * BOARD_SIZE + pos.x - pos.y + BOARD_SIZE - 1;  // ↘ : x - y constant
            default: return 4 * BOARD_SIZE - 1 + pos.x + pos.y;               // ↗ : x + y constant
        }
    }
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

int Board::stonesNearby(Pos pos) const {
    return nearbyStones_[index(pos)];
}

int Board::countStonesNearbyFromScratch(Pos pos) const {
    int count = 0;
    for (int dy = -CANDIDATE_RADIUS; dy <= CANDIDATE_RADIUS; dy++) {
        for (int dx = -CANDIDATE_RADIUS; dx <= CANDIDATE_RADIUS; dx++) {
            const Pos neighbor = {pos.x + dx, pos.y + dy};
            if (isInside(neighbor) && at(neighbor) != Cell::Empty)
                count++;
        }
    }
    return count;
}

void Board::putStone(Pos pos, Cell color) {
    cells_[index(pos)] = color;
    hash_ ^= ZOBRIST.stones[index(pos)][colorIndex(color)];
    addNearby(pos, 1);
    rescoreLines(pos);
}

void Board::clearStone(Pos pos) {
    hash_ ^= ZOBRIST.stones[index(pos)][colorIndex(at(pos))];
    cells_[index(pos)] = Cell::Empty;
    addNearby(pos, -1);
    rescoreLines(pos);
}

// Comme le hash : chaque pose ou retrait corrige les 25 cases autour, undo défait tout seul.
void Board::addNearby(Pos pos, int delta) {
    for (int dy = -CANDIDATE_RADIUS; dy <= CANDIDATE_RADIUS; dy++) {
        for (int dx = -CANDIDATE_RADIUS; dx <= CANDIDATE_RADIUS; dx++) {
            const Pos neighbor = {pos.x + dx, pos.y + dy};
            if (isInside(neighbor))
                nearbyStones_[index(neighbor)] = static_cast<uint8_t>(nearbyStones_[index(neighbor)] + delta);
        }
    }
}

void Board::addCaptures(Cell color, int delta) {
    int& count = captureCounts_[colorIndex(color)];
    hash_ ^= ZOBRIST.captures[colorIndex(color)][count];
    count += delta;
    assert(count >= 0 && count <= MAX_CAPTURE_COUNT && "compteur de captures hors limites");
    hash_ ^= ZOBRIST.captures[colorIndex(color)][count];
}

int Board::runScore(Cell color) const {
    return runScores_[colorIndex(color)];
}

int Board::captureThreats(Cell color) const {
    return captureThreats_[colorIndex(color)];
}

// Seules les 4 lignes qui passent par pos ont changé : on retire leur ancienne valeur, on ajoute la nouvelle.
void Board::rescoreLines(Pos pos) {
    for (int d = 0; d < static_cast<int>(DIRECTIONS.size()); d++) {
        LineValue& old = lineValues_[lineIndex(pos, d)];
        const LineValue now = scoreLine(pos, DIRECTIONS[d]);
        for (int c = 0; c < 2; c++) {
            runScores_[c] += now.runs[c] - old.runs[c];
            captureThreats_[c] += now.threats[c] - old.threats[c];
        }
        old = now;
    }
}

// Mêmes règles que le recalcul complet d'Eval. Suites : une suite de 5 ou plus compte toujours, une suite plus
// courte compte une fois par extrémité vide. Menaces : X O O _ ou _ O O X, X peut prendre la paire O O.
Board::LineValue Board::scoreLine(Pos pos, Pos dir) const {
    Pos start = pos;
    while (isInside(step(start, dir, -1)))
        start = step(start, dir, -1);
    std::array<Cell, BOARD_SIZE> line{};
    int size = 0;
    for (Pos cur = start; isInside(cur); cur = step(cur, dir))
        line[size++] = at(cur);

    LineValue value;
    for (int i = 0; i < size;) {
        const Cell color = line[i];
        if (color == Cell::Empty) {
            i++;
            continue;
        }
        const int begin = i;
        while (i < size && line[i] == color)
            i++;
        const int length = i - begin;
        int score = runWeight(length);
        if (length < 5)
            score *= (begin > 0 && line[begin - 1] == Cell::Empty) + (i < size && line[i] == Cell::Empty);
        value.runs[colorIndex(color)] += score;
    }

    for (int i = 0; i + 3 < size; i++) {
        const Cell pair = line[i + 1];
        if (pair == Cell::Empty || line[i + 2] != pair)
            continue;
        const Cell taker = opponent(pair);
        if ((line[i] == taker && line[i + 3] == Cell::Empty) || (line[i] == Cell::Empty && line[i + 3] == taker))
            value.threats[colorIndex(taker)]++;
    }
    return value;
}
