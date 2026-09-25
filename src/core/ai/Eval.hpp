#pragma once

#include "app/Dto.hpp"
#include "core/Game.hpp"

namespace Eval {

// Score positif = avantage pour color.
int evaluate(const Game& game, Cell color);

}  // namespace Eval
