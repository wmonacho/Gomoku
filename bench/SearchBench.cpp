// Bench de la recherche : parties complètes IA contre IA, compilées comme le vrai jeu (sans assert).
// Pas demandé par le sujet : c'est notre alarme pour la profondeur 10 et la moyenne < 500 ms.
//   make bench                    limite de temps de GameService
//   make bench BENCH_ARGS=1000    autre limite, en ms

#include "core/Game.hpp"
#include "core/ai/Search.hpp"
#include "core/ai/SearchConfig.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

namespace {

    constexpr double DEFAULT_TIME_LIMIT_MS = SearchConfig::TIME_LIMIT_MS;
    constexpr int    DEPTH = SearchConfig::DEPTH;

    constexpr int    MAX_MOVES_PER_GAME = 60;
    constexpr int    OPENING_MOVES_COUNTED = 10;
    // Critère du sujet (chap. II) : plus de 0,5 s en moyenne, projet non validé.
    constexpr double MAX_AVERAGE_MS = 500.0;

    struct MoveStats {
        int    game;
        int    move;
        double elapsedMs;
        int    depth;
        long   nodes;
        bool   timeUp;
        bool   forcedWin;
    };

    // Coups joués avant de laisser l'IA continuer, pour que les parties ne se ressemblent pas toutes.
    const std::vector<std::vector<Pos>> OPENINGS = {
        {},
        {{9, 9}, {10, 10}},
        {{9, 9}, {9, 10}},
        {{8, 8}, {10, 9}},
        {{9, 9}, {11, 11}, {10, 8}},
    };

    std::vector<MoveStats> playGame(int gameIndex, const std::vector<Pos>& opening, double timeLimitMs) {
        Game game;
        // La table de transposition fait plusieurs dizaines de Mo : pas sur la pile.
        auto search = std::make_unique<Search>();
        for (const Pos& pos : opening)
            game.play(pos);

        std::vector<MoveStats> stats;
        while (!game.winner() && static_cast<int>(stats.size()) < MAX_MOVES_PER_GAME) {
            const MoveSuggestion suggestion = search->findBestMove(game, DEPTH, timeLimitMs);
            if (!game.play(suggestion.pos).isLegal) {
                std::printf("  partie %d : coup illégal (%d, %d) proposé par l'IA\n",
                            gameIndex, suggestion.pos.x, suggestion.pos.y);
                std::exit(1);
            }
            stats.push_back({gameIndex, static_cast<int>(stats.size()) + 1,
                             suggestion.elapsedMs, suggestion.depth, suggestion.nodesEvaluated,
                             suggestion.stopReason == StopReason::TimeUp, suggestion.forcedWin});
        }
        return stats;
    }

    double averageMs(const std::vector<MoveStats>& stats) {
        double total = 0.0;
        for (const MoveStats& s : stats)
            total += s.elapsedMs;
        return stats.empty() ? 0.0 : total / stats.size();
    }

    // Sous DEPTH faute de temps, sans victoire déjà assurée : c'est ce qui fait échouer le sujet.
    bool tooShallow(const MoveStats& s) {
        return s.depth < DEPTH && s.timeUp && !s.forcedWin;
    }
}

int main(int argc, char** argv) {
    const double timeLimitMs = argc > 1 ? std::atof(argv[1]) : DEFAULT_TIME_LIMIT_MS;
    if (timeLimitMs <= 0.0) {
        std::printf("limite de temps invalide : %s\n", argv[1]);
        return 1;
    }
    std::printf("Bench de la recherche : profondeur %d, limite %.0f ms, %zu parties\n\n",
                DEPTH, timeLimitMs, OPENINGS.size());

    std::vector<MoveStats> all;
    std::vector<MoveStats> openings;
    for (size_t i = 0; i < OPENINGS.size(); i++) {
        const std::vector<MoveStats> stats = playGame(static_cast<int>(i) + 1, OPENINGS[i], timeLimitMs);
        std::printf("  partie %zu : %2zu coups, moyenne %6.1f ms\n", i + 1, stats.size(), averageMs(stats));
        all.insert(all.end(), stats.begin(), stats.end());
        for (const MoveStats& s : stats)
            if (s.move <= OPENING_MOVES_COUNTED)
                openings.push_back(s);
    }
    if (all.empty()) {
        std::printf("aucun coup joué\n");
        return 1;
    }

    double maxMs = 0.0;
    long   totalNodes = 0;
    int    totalDepth = 0;
    int    minDepth = DEPTH;
    for (const MoveStats& s : all) {
        maxMs = std::max(maxMs, s.elapsedMs);
        totalNodes += s.nodes;
        totalDepth += s.depth;
        minDepth = std::min(minDepth, s.depth);
    }

    std::printf("\nCoups sous la profondeur %d :\n", DEPTH);
    int shallowMoves = 0;
    int timeouts = 0;
    int forcedWins = 0;
    for (const MoveStats& s : all) {
        if (s.depth >= DEPTH)
            continue;
        const bool timeout = tooShallow(s);
        shallowMoves++;
        timeouts += timeout;
        forcedWins += s.forcedWin;
        // « autre » : ni temps écoulé ni victoire forcée, seul MAX_PLY peut l'expliquer.
        std::printf("  partie %d, coup %2d : profondeur %2d, %6.1f ms, %s\n", s.game, s.move, s.depth,
                    s.elapsedMs, s.forcedWin ? "victoire forcée trouvée" : timeout ? "temps écoulé" : "autre");
    }
    if (shallowMoves == 0)
        std::printf("  aucun\n");

    const double average = averageMs(all);
    std::printf("\nBilan sur %zu coups :\n", all.size());
    std::printf("  temps moyen                 %6.1f ms   (critère : < %.0f ms)\n", average, MAX_AVERAGE_MS);
    std::printf("  temps moyen des %d premiers %6.1f ms   (le pire cas : partie courte)\n",
                OPENING_MOVES_COUNTED, averageMs(openings));
    std::printf("  temps maximum               %6.1f ms\n", maxMs);
    std::printf("  profondeur moyenne          %6.2f\n", static_cast<double>(totalDepth) / all.size());
    std::printf("  profondeur minimale         %6d\n", minDepth);
    std::printf("  nœuds par coup              %6ld\n", totalNodes / static_cast<long>(all.size()));
    std::printf("  sous %d : temps écoulé      %6d\n", DEPTH, timeouts);
    std::printf("  sous %d : victoire forcée   %6d\n", DEPTH, forcedWins);

    const bool ok = average < MAX_AVERAGE_MS && timeouts == 0;
    std::printf("\n%s\n", ok ? "OK : moyenne < 500 ms et profondeur 10 à chaque coup (hors victoire forcée)"
                             : "ÉCHEC : voir ci-dessus");
    return ok ? 0 : 1;
}
