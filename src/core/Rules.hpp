#pragma once

#include <vector>

#include "app/Dto.hpp"
#include "core/Board.hpp"

namespace Rules {

constexpr int WINNING_CAPTURES = 10;

std::vector<Pos> findCaptures(const Board& board, Pos pos, Cell color);
bool             makesFiveInARow(const Board& board, Pos pos, Cell color);
bool             hasFiveInARow(const Board& board, Cell color);
bool             canAnswerFive(const Board& board, Pos fivePos, Cell fiveColor);
bool             createsDoubleFreeThree(const Board& board, Pos pos, Cell color);

}  // namespace Rules
