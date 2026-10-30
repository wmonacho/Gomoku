#pragma once

#include "app/Dto.hpp"
#include "core/Game.hpp"

class Search {
public:
    Search();

    // Modifie temporairement game (play/undo), mais le rend dans son état initial.
    MoveSuggestion findBestMove(Game& game, int maxDepth, double timeLimitMs);

private:
};
