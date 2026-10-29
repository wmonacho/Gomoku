#pragma once

#include "app/Dto.hpp"
#include "core/Game.hpp"
#include "core/ai/Search.hpp"

class GameService {
public:
    GameService();

    // difficulty = profondeur maximale de l'IA, en coups.
    void           newGame(Mode mode, int difficulty, GameInfo& gameInfo);
    MoveResult     playMove(Pos pos);
    // Meilleur coup pour le joueur courant, sans le jouer : l'appelant enchaîne avec playMove.
    MoveSuggestion suggestMove();
    GameView       currentView() const;
    bool           isLegalMove(Pos pos) const;

private:
    Game game_;
    Mode mode_;
    int  difficulty_;
    Search search_;
};
