#pragma once

#include "app/Dto.hpp"
#include "core/Game.hpp"
#include "core/ai/Search.hpp"

class GameService {
public:
    GameService();

    // humanColor n'est utilisé qu'en mode PvAI.
    void       newGame(Mode mode, Cell humanColor);
    MoveResult playHumanMove(Pos pos);
    AiMove     playAiMove();
    
    // Meilleur coup pour le joueur courant, sans le jouer (aide au joueur humain).
    Pos        suggestMove();
    GameView   currentView() const;
    bool       isLegalMove(Pos pos) const;

private:
    Game game_;
    Mode mode_;
    Cell humanColor_;
    Search search_;
};
