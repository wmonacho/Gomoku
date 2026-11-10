#include "core/ai/Search.hpp"

#include "core/Board.hpp"
#include "core/Rules.hpp"
#include "core/ai/Eval.hpp"

#include <algorithm>
#include <cassert>
#include <new>
#include <utility>
#include <vector>

namespace {

    using Clock = std::chrono::steady_clock;

    // Large près de la racine, étroit en bas de l'arbre, là où il y a le plus de nœuds.
    constexpr int      ROOT_CANDIDATES = 10;
    constexpr int      MIN_CANDIDATES = 4;
    constexpr int      INF = 1'000'000'000;
    // Puissances de 2 : l'index est key & (taille - 1).
    constexpr size_t   TABLE_SIZE = 1 << 20;
    constexpr size_t   MIN_TABLE_SIZE = 1 << 10;
    constexpr long     NODES_BETWEEN_CLOCK_CHECKS = 256;
    constexpr uint64_t PENDING_FIVE_KEY = 0x9E3779B97F4A7C15ULL;

    constexpr int TT_MOVE_BONUS = 1'000'000;
    constexpr int KILLER_BONUS = 5'000;
    constexpr int CAPTURE_BONUS = 500;

    // Dernier recours si aucun candidat n'est légal (double-trois partout autour des pierres).
    std::vector<Pos> findAnyLegalMove(const Game& game) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                if (game.isLegalMove({x, y}))
                    return {{x, y}};
            }
        }
        return {};
    }

    // Cases vides proches des pierres, lues dans le compteur de Board. La légalité (double-trois) se vérifie
    // après le tri : c'est le test le plus cher.
    std::vector<Pos> generateCandidates(const Game& game) {
        const Board& board = game.board();
        std::vector<Pos> candidates;
        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                const Pos pos = {x, y};
                if (board.at(pos) == Cell::Empty && board.stonesNearby(pos) > 0)
                    candidates.push_back(pos);
            }
        }

        const Pos center = {BOARD_SIZE / 2, BOARD_SIZE / 2};
        if (candidates.empty() && board.at(center) == Cell::Empty)
            return {center};
        return candidates;
    }

    // Longueur des suites collées à pos dans la direction dir : runs[0] pour Noir, runs[1] pour Blanc.
    // D'un côté, la première pierre décide de la couleur de la suite : l'autre couleur y vaut forcément 0.
    std::array<int, 2> runLengths(const Board& board, Pos pos, Pos dir) {
        std::array<int, 2> runs = {0, 0};
        for (const Pos& d : {dir, Pos{-dir.x, -dir.y}}) {
            Pos cur = step(pos, d);
            if (!board.isInside(cur) || board.at(cur) == Cell::Empty)
                continue;
            const Cell color = board.at(cur);
            int& length = runs[static_cast<int>(color) - 1];
            while (board.isInside(cur) && board.at(cur) == color) {
                length++;
                cur = step(cur, d);
            }
        }
        return runs;
    }

    // Barème du tri des coups (pas celui de l'évaluation) : une case qui prolonge une suite de 4 passe devant tout.
    int runScore(int length) {
        constexpr std::array<int, 5> SCORES = {0, 1, 10, 100, 10'000};
        return SCORES[std::min(length, 4)];
    }

    // Note rapide : aligner ses pierres compte double, bloquer celles de l'adversaire compte simple.
    // Prendre une paire, ou jouer là où l'adversaire en prendrait une, passe aussi devant : sans ça, la défense
    // contre une capture sortait des 4 à 10 coups essayés et l'IA perdait ses paires même en cherchant loin.
    int moveScore(const Board& board, Pos pos, Cell color) {
        const int mine = static_cast<int>(color) - 1;
        int score = 0;
        for (const Pos& dir : DIRECTIONS) {
            const std::array<int, 2> runs = runLengths(board, pos, dir);
            score += 2 * runScore(runs[mine]);
            score += runScore(runs[1 - mine]);
        }
        if (Rules::canCapture(board, pos, color))
            score += CAPTURE_BONUS;
        if (Rules::canCapture(board, pos, opponent(color)))
            score += CAPTURE_BONUS;
        return score;
    }

    size_t candidateLimit(int ply) {
        return static_cast<size_t>(std::max(MIN_CANDIDATES, ROOT_CANDIDATES - ply));
    }

    // Clé de la table : le hash du plateau, plus le 5 en sursis s'il y en a un. Mêmes pierres avec ou sans
    // 5 en sursis, ce ne sont pas les mêmes positions (dans l'une, l'adversaire doit casser le 5 tout de suite).
    uint64_t keyOf(const Game& game) {
        const std::optional<Pos> pending = game.pendingFive();
        if (!pending)
            return game.board().hash();
        const uint64_t cell = pending->y * BOARD_SIZE + pending->x + 1;
        return game.board().hash() ^ (cell * PENDING_FIVE_KEY);
    }
}

// Le sujet interdit de planter, même sans mémoire : si la table est refusée, on retente deux fois plus petit.
Search::Search() {
    for (size_t size = TABLE_SIZE; size >= MIN_TABLE_SIZE && table_.empty(); size /= 2) {
        try {
            table_.resize(size);
        } catch (const std::bad_alloc&) {
            // resize n'a rien changé : on passe à la taille suivante.
        }
    }
    clear();
}

Search::Entry& Search::entryFor(uint64_t key) {
    if (table_.empty())
        return fallbackEntry_;
    return table_[key & (table_.size() - 1)];
}

void Search::clear() {
    std::fill(table_.begin(), table_.end(), Entry{});
    fallbackEntry_ = Entry{};
    for (auto& killers : killers_)
        killers.fill(NO_MOVE);
}

bool Search::outOfTime() {
    if (nodes_ % NODES_BETWEEN_CLOCK_CHECKS == 0
        && (Clock::now() >= deadline_ || (stop_ && stop_->load(std::memory_order_relaxed))))
        timeUp_ = true;
    return timeUp_;
}

void Search::rememberKiller(int ply, Pos move) {
    if (killers_[ply][0] == move)
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
        if (move == ttMove)
            score += TT_MOVE_BONUS;
        else if (move == killers_[ply][0] || move == killers_[ply][1])
            score += KILLER_BONUS;
        scored.push_back({score, move});
    }

    // À note égale, l'ordre des cases tranche : le résultat ne dépend pas de l'ordre de generateCandidates.
    std::sort(scored.begin(), scored.end(), [](const auto& a, const auto& b) {
        if (a.first != b.first)
            return a.first > b.first;
        return a.second.y * BOARD_SIZE + a.second.x < b.second.y * BOARD_SIZE + b.second.x;
    });

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

    // Partie finie : score énorme, plus grand si la victoire arrive tôt (il reste plus de profondeur).
    if (game.winner()) {
        const int score = WINNING_SCORE + depth;
        return *game.winner() == game.currentPlayer() ? score : -score;
    }
    // Profondeur épuisée : on note la position avec l'heuristique.
    if (depth == 0 || ply >= MAX_PLY)
        return Eval::evaluate(game, game.currentPlayer());

    // Position déjà cherchée au moins aussi profond : la table donne le score sans refaire la recherche.
    const uint64_t key = keyOf(game);
    Entry& entry = entryFor(key);
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

    // Coups à essayer, les plus prometteurs d'abord : l'alpha-bêta coupe d'autant plus tôt.
    const std::vector<Pos> moves = orderMoves(game, ply, ttMove);
    if (moves.empty())
        return Eval::evaluate(game, game.currentPlayer());

    // Negamax : le score d'un coup pour moi est l'opposé du score de la position pour l'adversaire.
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
        // L'adversaire a déjà mieux ailleurs : il ne nous laissera pas arriver ici, inutile de continuer.
        if (alpha >= beta) {
            rememberKiller(ply, move);
            break;
        }
    }

    // Exact si le score est dans la fenêtre ; sinon c'est seulement un plafond (Upper) ou un plancher (Lower).
    const Bound bound = best <= originalAlpha ? Bound::Upper
                      : best >= beta          ? Bound::Lower
                                              : Bound::Exact;
    entry = {key, depth, best, bound, bestMove};
    return best;
}

int Search::searchRoot(Game& game, const std::vector<Pos>& moves, int depth, Pos& bestMove,
                       std::vector<RootMove>& rootMoves) {
    rootMoves.clear();
    int alpha = -INF;
    for (const Pos& move : moves) {
        game.playLegal(move);
        const int score = -negamax(game, depth - 1, 1, -INF, -alpha);
        game.undo();
        if (timeUp_)
            return alpha;
        // Fenêtre (alpha, +infini) : au-dessus d'alpha le score est exact, sinon c'est seulement un plafond.
        rootMoves.push_back({move, score, score > alpha});
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
        const Entry& entry = entryFor(key);
        if (entry.key != key || !game.isLegalMove(entry.bestMove))
            break;
        line.push_back(entry.bestMove);
        game.playLegal(entry.bestMove);
    }
    for (size_t i = 0; i < line.size(); i++)
        game.undo();
    return line;
}

MoveSuggestion Search::findBestMove(Game& game, int maxDepth, double timeLimitMs, const std::atomic<bool>* stop) {
    assert(maxDepth > 0 && "findBestMove() sans profondeur");
    assert(timeLimitMs > 0 && "findBestMove() sans temps de réflexion");
    assert(!game.isOver() && "findBestMove() sur une partie déjà terminée");
    // Une partie compte au plus 361 poses + 16 reprises (8 pierres par joueur : à 10, elle est finie), et la
    // recherche ajoute au plus MAX_PLY coups : l'historique de Board ne peut pas déborder.
    static_assert(BOARD_CELLS + 2 * (Rules::WINNING_CAPTURES - 2) + MAX_PLY <= MAX_HISTORY,
                  "MAX_HISTORY trop petit pour une partie complète plus une recherche");

    const Clock::time_point start = Clock::now();
    deadline_ = start + std::chrono::duration_cast<Clock::duration>(
                            std::chrono::duration<double, std::milli>(timeLimitMs));
    timeUp_ = false;
    nodes_ = 0;
    stop_ = stop;

    std::vector<Pos> moves = orderMoves(game, 0, NO_MOVE);
    if (moves.empty())
        return {NO_MOVE, 0.0, 0, 0, {}, StopReason::MaxDepth, false, 0, {}};

    Pos bestMove = moves[0];
    int bestScore = 0;
    int completedDepth = 0;
    StopReason stopReason = StopReason::MaxDepth;
    std::vector<RootMove> rootMoves;
    std::vector<RootMove> iterationMoves;
    for (int depth = 1; depth <= maxDepth && depth < MAX_PLY; depth++) {
        Pos move = bestMove;
        const int score = searchRoot(game, moves, depth, move, iterationMoves);
        if (timeUp_) {
            stopReason = StopReason::TimeUp;
            break;
        }

        bestMove = move;
        bestScore = score;
        completedDepth = depth;
        rootMoves.swap(iterationMoves);
        // Le meilleur coup passe en tête : au tour suivant, il est essayé en premier.
        const auto found = std::find(moves.begin(), moves.end(), bestMove);
        std::rotate(moves.begin(), found, found + 1);
    }

    stop_ = nullptr;
    // Du meilleur au moins bon ; à score égal, un score exact passe devant un plafond.
    std::stable_sort(rootMoves.begin(), rootMoves.end(), [](const RootMove& a, const RootMove& b) {
        if (a.score != b.score)
            return a.score > b.score;
        return a.exact && !b.exact;
    });
    const double elapsedMs = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
    return {bestMove, elapsedMs, completedDepth, nodes_,
            principalVariation(game, bestMove, completedDepth),
            stopReason, bestScore >= WINNING_SCORE, bestScore, rootMoves};
}
