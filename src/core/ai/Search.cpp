#include "core/ai/Search.hpp"

#include "core/ai/Eval.hpp"

#include <cassert>

namespace {

    constexpr int CANDIDATE_RADIUS = 2;

    bool hasStoneNearby(const Board& board, Pos pos) {
        for (int dy = -CANDIDATE_RADIUS; dy <= CANDIDATE_RADIUS; dy++) {
            for (int dx = -CANDIDATE_RADIUS; dx <= CANDIDATE_RADIUS; dx++) {
                const Pos neighbor = {pos.x + dx, pos.y + dy};
                if (board.isInside(neighbor) && board.at(neighbor) != Cell::Empty)
                    return true;
            }
        }
        return false;
    }

    [[maybe_unused]] std::vector<Pos> generateCandidates(const Game& game) {
        const Board& board = game.board();
        std::vector<Pos> candidates;
        bool boardIsEmpty = true;

        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                const Pos pos = {x, y};
                if (board.at(pos) != Cell::Empty)
                    boardIsEmpty = false;
                else if (hasStoneNearby(board, pos) && game.isLegalMove(pos))
                    candidates.push_back(pos);
            }
        }

        if (boardIsEmpty)
            return {{BOARD_SIZE / 2, BOARD_SIZE / 2}};
        return candidates;
    }
}



// coups légaux proches des pierres existantes (rayon 1-2), triés pour élaguer plus tôt.
// - alphaBeta(game, depth, alpha, beta, ...) : negamax avec play/undo, Eval::evaluate aux feuilles
//   et quand winner() est défini.

Search::Search() {
    // TODO: initialiser la table de transposition (hash Zobrist de la position -> score, profondeur, meilleur coup).
}

MoveSuggestion Search::findBestMove(Game& game, int maxDepth, double timeLimitMs) {
    assert(maxDepth > 0 && "findBestMove() sans profondeur");
    assert(timeLimitMs > 0 && "findBestMove() sans temps de réflexion");
    assert(!game.winner() && "findBestMove() sur une partie déjà terminée");
    // TODO: approfondissement itératif :
    // 1. Démarrer le chrono.
    // 2. depth = 1, 2, 3... jusqu'à maxDepth : lancer alphaBeta, garder le meilleur coup de la dernière profondeur complète.
    // 3. S'arrêter avant timeLimitMs même si maxDepth n'est pas atteint (le sujet exige en moyenne < 0,5 s par coup).
    // 4. Remplir MoveSuggestion : pos, elapsedMs, depth, nodesEvaluated, principalVariation.
    return {};
}
