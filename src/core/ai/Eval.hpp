#pragma once

#include "app/Dto.hpp"
#include "core/Game.hpp"

namespace Eval {

// Score positif = avantage pour color.
int evaluate(const Game& game, Cell color);
// Même résultat qu'evaluate, en reparcourant tout le plateau : sert à vérifier le score incrémental dans les tests.
int evaluateFromScratch(const Game& game, Cell color);

}  // namespace Eval
