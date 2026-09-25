#include "core/ai/Search.hpp"

#include "core/ai/Eval.hpp"

// TODO: dans un namespace anonyme :
// - generateCandidates(game) : coups légaux proches des pierres existantes (rayon 1-2), triés pour élaguer plus tôt.
// - alphaBeta(game, depth, alpha, beta, ...) : negamax avec play/undo, Eval::evaluate aux feuilles
//   et quand winner() est défini.

Search::Search() {
    // TODO: initialiser la table de transposition (hash Zobrist de la position -> score, profondeur, meilleur coup).
}

AiMove Search::findBestMove(Game& game, double timeLimitMs) {
    // TODO: approfondissement itératif :
    // 1. Démarrer le chrono.
    // 2. depth = 1, 2, 3... : lancer alphaBeta, garder le meilleur coup de la dernière profondeur complète.
    // 3. S'arrêter avant timeLimitMs (le sujet exige en moyenne < 0,5 s par coup).
    // 4. Remplir AiMove : pos, elapsedMs, depth, nodesEvaluated, principalVariation.
    return {};
}
