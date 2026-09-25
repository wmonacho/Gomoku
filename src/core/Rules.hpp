#pragma once

#include <vector>

#include "app/Dto.hpp"
#include "core/Board.hpp"

namespace Rules {

std::vector<Pos> findCaptures(const Board& board, Pos pos, Cell color);
bool             makesFiveInARow(const Board& board, Pos pos, Cell color);
bool             createsDoubleFreeThree(const Board& board, Pos pos, Cell color);

}  // namespace Rules
