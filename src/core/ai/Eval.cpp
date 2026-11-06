#include "core/ai/Eval.hpp"

#include <cassert>

namespace {
    constexpr int CAPTURE_WEIGHT = 50;

    int runWeight(int length) {
        if (length >= 5)
            return 10'000;
        if (length == 4)
            return 1'000;
        if (length == 3)
            return 100;
        if (length == 2)
            return 10;
        return 0;
    }

    bool holds(const Board& board, Pos pos, Cell content) {
        return board.isInside(pos) && board.at(pos) == content;
    }

    int scoreRun(const Board& board, Pos start, Pos dir, Cell color) {
        int length = 1;
        while (holds(board, step(start, dir, length), color))
            length++;

        if (length >= 5)
            return runWeight(length);

        const int openEnds = holds(board, step(start, dir, -1), Cell::Empty)
                           + holds(board, step(start, dir, length), Cell::Empty);
        return runWeight(length) * openEnds;
    }

    int scoreColor(const Game& game, Cell color) {
        const Board& board = game.board();
        int score = game.captureCount(color) * CAPTURE_WEIGHT;

        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                const Pos pos = {x, y};
                if (board.at(pos) != color)
                    continue;
                for (const Pos& dir : DIRECTIONS)
                    if (!holds(board, step(pos, dir, -1), color))
                        score += scoreRun(board, pos, dir, color);
            }
        }
        return score;
    }
}


namespace Eval {

int evaluate(const Game& game, Cell color) {
    assert(color != Cell::Empty && "evaluate() demande le point de vue d'un joueur, pas Empty");

    return scoreColor(game, color) - scoreColor(game, opponent(color));
}

}
