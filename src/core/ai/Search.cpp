#include "core/ai/Search.hpp"

#include "core/Board.hpp"
#include "core/Rules.hpp"
#include "core/ai/Eval.hpp"

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

namespace {

    using Clock = std::chrono::steady_clock;

    constexpr int      CANDIDATE_RADIUS = 2;
    // Large près de la racine, étroit en bas de l'arbre, là où il y a le plus de nœuds.
    constexpr int      ROOT_CANDIDATES = 10;
    constexpr int      MIN_CANDIDATES = 4;
    constexpr int      INF = 1'000'000'000;
    constexpr int      WINNING_SCORE = 100'000'000;
    constexpr size_t   TABLE_SIZE = 1 << 20;
    constexpr long     NODES_BETWEEN_CLOCK_CHECKS = 256;
    constexpr uint64_t PENDING_FIVE_KEY = 0x9E3779B97F4A7C15ULL;

    constexpr int TT_MOVE_BONUS = 1'000'000;
    constexpr int KILLER_BONUS = 5'000;
    constexpr int CAPTURE_BONUS = 500;

    bool samePos(Pos a, Pos b) {
        return a.x == b.x && a.y == b.y;
    }

    std::vector<Pos> findAnyLegalMove(const Game& game) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                if (game.isLegalMove({x, y}))
                    return {{x, y}};
            }
        }
        return {};
    }

    // Cases vides proches des pierres. La légalité (double-trois) se vérifie après le tri : c'est le test le plus cher.
    std::vector<Pos> generateCandidates(const Game& game) {
        const Board& board = game.board();
        std::array<bool, BOARD_CELLS> seen{};
        std::vector<Pos> candidates;
        bool boardIsEmpty = true;

        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                if (board.at({x, y}) == Cell::Empty)
                    continue;
                boardIsEmpty = false;
                for (int dy = -CANDIDATE_RADIUS; dy <= CANDIDATE_RADIUS; dy++) {
                    for (int dx = -CANDIDATE_RADIUS; dx <= CANDIDATE_RADIUS; dx++) {
                        const Pos neighbor = {x + dx, y + dy};
                        if (!board.isInside(neighbor) || seen[neighbor.y * BOARD_SIZE + neighbor.x])
                            continue;
                        seen[neighbor.y * BOARD_SIZE + neighbor.x] = true;
                        if (board.at(neighbor) == Cell::Empty)
                            candidates.push_back(neighbor);
                    }
                }
            }
        }

        if (boardIsEmpty)
            return {{BOARD_SIZE / 2, BOARD_SIZE / 2}};
        return candidates;
    }

    int runLength(const Board& board, Pos pos, Pos dir, Cell color) {
        int length = 0;
        for (const Pos& d : {dir, Pos{-dir.x, -dir.y}}) {
            Pos cur = step(pos, d);
            while (board.isInside(cur) && board.at(cur) == color) {
                length++;
                cur = step(cur, d);
            }
        }
        return length;
    }

    int runScore(int length) {
        constexpr std::array<int, 5> SCORES = {0, 1, 10, 100, 10'000};
        return SCORES[std::min(length, 4)];
    }

    // Note rapide : aligner ses pierres compte double, bloquer celles de l'adversaire compte simple.
    int moveScore(const Board& board, Pos pos, Cell color) {
        int score = 0;
        for (const Pos& dir : DIRECTIONS) {
            score += 2 * runScore(runLength(board, pos, dir, color));
            score += runScore(runLength(board, pos, dir, opponent(color)));
        }
        if (Rules::canCapture(board, pos, color))
            score += CAPTURE_BONUS;
        return score;
    }

    size_t candidateLimit(int ply) {
        return static_cast<size_t>(std::max(MIN_CANDIDATES, ROOT_CANDIDATES - ply));
    }

    uint64_t keyOf(const Game& game) {
        const std::optional<Pos> pending = game.pendingFive();
        if (!pending)
            return game.board().hash();
        const uint64_t cell = pending->y * BOARD_SIZE + pending->x + 1;
        return game.board().hash() ^ (cell * PENDING_FIVE_KEY);
    }
}

Search::Search() : table_(TABLE_SIZE) {
    clear();
}

void Search::clear() {
    std::fill(table_.begin(), table_.end(), Entry{});
    for (auto& killers : killers_)
        killers.fill(NO_MOVE);
}

bool Search::outOfTime() {
    if (nodes_ % NODES_BETWEEN_CLOCK_CHECKS == 0 && Clock::now() >= deadline_)
        timeUp_ = true;
    return timeUp_;
}

void Search::rememberKiller(int ply, Pos move) {
    if (samePos(killers_[ply][0], move))
        return;
    killers_[ply][1] = killers_[ply][0];
    killers_[ply][0] = move;
}

std::vector<Pos> Search::orderMoves(const Game& game, int ply, Pos ttMove) const {
    const std::vector<Pos> candidates = generateCandidates(game);
    std::vector<std::pair<int, Pos>> scored;
    scored.reserve(candidates.size());

    for (const Pos& move : candidates) {
        int score = moveScore(game.board(), move, game.currentPlayer());
        if (samePos(move, ttMove))
            score += TT_MOVE_BONUS;
        else if (samePos(move, killers_[ply][0]) || samePos(move, killers_[ply][1]))
            score += KILLER_BONUS;
        scored.push_back({score, move});
    }

    std::sort(scored.begin(), scored.end(), [](const auto& a, const auto& b) { return a.first > b.first; });

    std::vector<Pos> moves;
    const size_t limit = candidateLimit(ply);
    moves.reserve(limit);
    for (const auto& [score, move] : scored) {
        if (game.isLegalMove(move))
            moves.push_back(move);
        if (moves.size() == limit)
            break;
    }
    if (moves.empty())
        return findAnyLegalMove(game);
    return moves;
}

int Search::negamax(Game& game, int depth, int ply, int alpha, int beta) {
    nodes_++;
    if (outOfTime())
        return 0;

    // if negamax find a win possibility, he return a big score
    if (game.winner()) {
        const int score = WINNING_SCORE + depth;
        return *game.winner() == game.currentPlayer() ? score : -score;
    }
    // if depth == 0 stop negamax and evaluate the pos
    if (depth == 0 || ply >= MAX_PLY)
        return Eval::evaluate(game, game.currentPlayer());

    const uint64_t key = keyOf(game);
    Entry& entry = table_[key % TABLE_SIZE];
    Pos ttMove = NO_MOVE;
    if (entry.key == key) {
        ttMove = entry.bestMove;
        if (entry.depth >= depth) {
            if (entry.bound == Bound::Exact)
                return entry.score;
            if (entry.bound == Bound::Lower && entry.score >= beta)
                return entry.score;
            if (entry.bound == Bound::Upper && entry.score <= alpha)
                return entry.score;
        }
    }

    // generate candidates for the next move, best ones first
    const std::vector<Pos> moves = orderMoves(game, ply, ttMove);
    if (moves.empty())
        return Eval::evaluate(game, game.currentPlayer());

    // try each candidate and return the BEST
    const int originalAlpha = alpha;
    int best = -INF;
    Pos bestMove = moves[0];
    for (const Pos& move : moves) {
        game.playLegal(move);
        const int score = -negamax(game, depth - 1, ply + 1, -beta, -alpha);
        game.undo();
        if (timeUp_)
            return 0;

        if (score > best) {
            best = score;
            bestMove = move;
        }
        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            rememberKiller(ply, move);
            break;
        }
    }

    const Bound bound = best <= originalAlpha ? Bound::Upper
                      : best >= beta          ? Bound::Lower
                                              : Bound::Exact;
    entry = {key, depth, best, bound, bestMove};
    return best;
}

int Search::searchRoot(Game& game, const std::vector<Pos>& moves, int depth, Pos& bestMove) {
    int alpha = -INF;
    for (const Pos& move : moves) {
        game.playLegal(move);
        const int score = -negamax(game, depth - 1, 1, -INF, -alpha);
        game.undo();
        if (timeUp_)
            return alpha;
        if (score > alpha) {
            alpha = score;
            bestMove = move;
        }
    }
    return alpha;
}

std::vector<Pos> Search::principalVariation(Game& game, Pos firstMove, int depth) {
    std::vector<Pos> line = {firstMove};
    game.playLegal(firstMove);
    while (static_cast<int>(line.size()) < depth && !game.winner()) {
        const uint64_t key = keyOf(game);
        const Entry& entry = table_[key % TABLE_SIZE];
        if (entry.key != key || !game.isLegalMove(entry.bestMove))
            break;
        line.push_back(entry.bestMove);
        game.playLegal(entry.bestMove);
    }
    for (size_t i = 0; i < line.size(); i++)
        game.undo();
    return line;
}

MoveSuggestion Search::findBestMove(Game& game, int maxDepth, double timeLimitMs) {
    assert(maxDepth > 0 && "findBestMove() sans profondeur");
    assert(timeLimitMs > 0 && "findBestMove() sans temps de réflexion");
    assert(!game.winner() && "findBestMove() sur une partie déjà terminée");

    const Clock::time_point start = Clock::now();
    deadline_ = start + std::chrono::duration_cast<Clock::duration>(
                            std::chrono::duration<double, std::milli>(timeLimitMs));
    timeUp_ = false;
    nodes_ = 0;

    std::vector<Pos> moves = orderMoves(game, 0, NO_MOVE);
    if (moves.empty())
        return {NO_MOVE, 0.0, 0, 0, {}, StopReason::MaxDepth, false};

    Pos bestMove = moves[0];
    int bestScore = 0;
    int completedDepth = 0;
    StopReason stopReason = StopReason::MaxDepth;
    for (int depth = 1; depth <= maxDepth && depth < MAX_PLY; depth++) {
        Pos move = bestMove;
        const int score = searchRoot(game, moves, depth, move);
        if (timeUp_) {
            stopReason = StopReason::TimeUp;
            break;
        }

        bestMove = move;
        bestScore = score;
        completedDepth = depth;
        const auto found = std::find_if(moves.begin(), moves.end(), [&](Pos m) { return samePos(m, bestMove); });
        std::rotate(moves.begin(), found, found + 1);
    }

    const double elapsedMs = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
    return {bestMove, elapsedMs, completedDepth, nodes_,
            principalVariation(game, bestMove, completedDepth),
            stopReason, bestScore >= WINNING_SCORE};
}
