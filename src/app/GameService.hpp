#pragma once

#include "app/Dto.hpp"
#include "core/Game.hpp"
#include "core/ai/Search.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <future>
#include <optional>

class GameService {
public:
    GameService();
    // Arrête une recherche en cours avant de détruire ce qu'elle utilise.
    ~GameService();

    // gameInfo.difficulty fixe la profondeur de l'IA pour toute la partie (voir SearchConfig::depthOf).
    void           newGame(GameInfo& gameInfo);
    // Refusé pendant que l'IA réfléchit : la partie ne doit pas changer sous la recherche.
    MoveResult     playMove(Pos pos);
    // Meilleur coup pour le joueur courant, sans le jouer (suggestion en hotseat). Toujours au niveau Difficile,
    // quel que soit le niveau de l'IA. Bloquant, jusqu'à SearchConfig::TIME_LIMIT_MS. NO_MOVE si la partie est
    // finie ou si l'IA réfléchit déjà.
    MoveSuggestion suggestMove();
    GameView       currentView() const;
    bool           isLegalMove(Pos pos) const;

    // Tour de l'IA, sans bloquer l'appelant :
    //   startAITurn()  lance la recherche dans un autre fil, sur une copie de la partie ;
    //   aiThinkingMs() donne le temps écoulé depuis, pour le chronomètre ;
    //   finishAITurn() joue le coup dès qu'il est prêt, et reste vide tant que la recherche tourne.
    bool                  isAITurn() const;
    bool                  isAIThinking() const;
    void                  startAITurn();
    double                aiThinkingMs() const;
    std::optional<AIMove> finishAITurn();
    // Version bloquante des trois précédentes (tests, bench).
    AIMove                playAITurn();

private:
    using Clock = std::chrono::steady_clock;

    void           cancelAITurn();
    MoveSuggestion fallbackSuggestion() const;

    Game                        game_;
    Mode                        mode_;  // PvP, PvAI ou AIvAI, choisi au menu
    int                         aiDepth_;
    bool                        debugLog_;
    std::array<bool, 2>         aiPlays_ = {false, false};  // index : couleur - 1
    Search                      search_;
    // Déclarés après search_ : le fil de recherche utilise les deux.
    std::atomic<bool>           stopSearch_{false};
    std::future<MoveSuggestion> pendingSearch_;
    Clock::time_point           thinkingSince_;
};
