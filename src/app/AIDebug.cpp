#include "app/AIDebug.hpp"

#include "core/ai/Search.hpp"

#include <cstdio>

namespace {

    std::string posText(Pos pos) {
        return "(" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")";
    }

    // Les scores de victoire valent plus de 100 millions : on les traduit plutôt que de les afficher.
    std::string scoreText(int score) {
        if (score >= Search::WINNING_SCORE)
            return "victoire forcée";
        if (score <= -Search::WINNING_SCORE)
            return "défaite forcée";
        return (score > 0 ? "+" : "") + std::to_string(score);
    }
}

namespace AIDebug {

std::string describe(const MoveSuggestion& suggestion) {
    if (suggestion.pos == NO_MOVE)
        return "[IA] aucun coup\n";

    char header[160];
    std::snprintf(header, sizeof(header), "[IA] coup %s : profondeur %d (%s), %ld nœuds, %.1f ms, score %s\n",
                  posText(suggestion.pos).c_str(), suggestion.depth,
                  suggestion.stopReason == StopReason::TimeUp ? "temps écoulé" : "profondeur max",
                  suggestion.nodesEvaluated, suggestion.elapsedMs, scoreText(suggestion.score).c_str());
    std::string text = header;

    text += "[IA] ligne prévue :";
    for (const Pos& pos : suggestion.principalVariation)
        text += " " + posText(pos);
    text += "\n";

    // « <= » : l'alpha-bêta a coupé ce coup dès qu'il a prouvé qu'il ne battait pas le meilleur.
    text += "[IA] coups étudiés :";
    for (size_t i = 0; i < suggestion.rootMoves.size(); i++) {
        const RootMove& move = suggestion.rootMoves[i];
        text += (i == 0 ? " " : " | ") + posText(move.pos) + " " + (move.exact ? "" : "<= ") + scoreText(move.score);
    }
    text += "\n";
    return text;
}

}  // namespace AIDebug
