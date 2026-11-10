#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <vector>

#include "app/Dto.hpp"
#include "core/Game.hpp"

class Search {
public:
    // Score d'une victoire forcée (plus la profondeur restante, pour préférer la plus rapide).
    static constexpr int WINNING_SCORE = 100'000'000;

    Search();

    // Modifie temporairement game (play/undo), mais le rend dans son état initial.
    // stop : posé à true depuis un autre fil, il arrête la recherche comme un temps écoulé (meilleur coup déjà trouvé).
    MoveSuggestion findBestMove(Game& game, int maxDepth, double timeLimitMs,
                                const std::atomic<bool>* stop = nullptr);
    // À chaque nouvelle partie : le hash suit la parité des coups, pas le joueur qui a commencé.
    void           clear();

private:
    enum class Bound : uint8_t { Exact, Lower, Upper };

    struct Entry {
        uint64_t key = 0;
        int      depth = -1;
        int      score = 0;
        Bound    bound = Bound::Exact;
        Pos      bestMove = NO_MOVE;
    };

    static constexpr int MAX_PLY = 64;

    int              searchRoot(Game& game, const std::vector<Pos>& moves, int depth, Pos& bestMove,
                                std::vector<RootMove>& rootMoves);
    int              negamax(Game& game, int depth, int ply, int alpha, int beta);
    std::vector<Pos> orderMoves(const Game& game, int ply, Pos ttMove) const;
    std::vector<Pos> principalVariation(Game& game, Pos firstMove, int depth);
    bool             outOfTime();
    void             rememberKiller(int ply, Pos move);
    Entry&           entryFor(uint64_t key);

    std::vector<Entry>                      table_;
    // Remplace la table si la mémoire a manqué : la recherche reste juste, seulement plus lente.
    Entry                                   fallbackEntry_;
    const std::atomic<bool>*                stop_ = nullptr;
    std::array<std::array<Pos, 2>, MAX_PLY> killers_;
    std::chrono::steady_clock::time_point   deadline_;
    bool                                    timeUp_ = false;
    long                                    nodes_ = 0;
};
