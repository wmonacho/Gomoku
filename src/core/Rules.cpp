#include "core/Rules.hpp"

#include <cassert>
#include <string>
#include <string_view>

namespace {

    int countInDirection(const Board& board, Pos pos, Pos dir, Cell color) {
        int count = 0;
        Pos cur = step(pos, dir);
        while (board.isInside(cur) && board.at(cur) == color) {
            ++count;
            cur = step(cur, dir);
        }
        return count;
    }

    bool checkPatternCapture(const Board& board, Pos pos, Pos dir, Cell color) {
        Cell enemyColor = (color == Cell::Black) ? Cell::White : Cell::Black;
        Pos pos1 = step(pos, dir);
        Pos pos2 = step(pos1, dir);
        Pos pos3 = step(pos2, dir);

        if (board.isInside(pos1) && board.isInside(pos2) && board.isInside(pos3)) {
            if (board.at(pos1) == enemyColor && board.at(pos2) == enemyColor && board.at(pos3) == color) {
                return true;
            }
        }
        return false;
    }

    std::vector<Pos> stonesOfFives(const Board& board, Pos pos, Cell color) {
        std::vector<Pos> stones;
        for (const Pos& dir : DIRECTIONS) {
            const Pos opposite = {-dir.x, -dir.y};
            const int forward = countInDirection(board, pos, dir, color);
            const int backward = countInDirection(board, pos, opposite, color);
            if (1 + forward + backward < 5)
                continue;
            for (int i = -backward; i <= forward; i++)
                stones.push_back(step(pos, dir, i));
        }
        return stones;
    }

    // Un 6 ou plus peut perdre une pierre au bout et rester un 5 : on revérifie chaque pierre restante.
    bool fiveSurvives(const Board& board, const std::vector<Pos>& stones, Cell color) {
        for (const Pos& stone : stones)
            if (board.at(stone) == color && Rules::makesFiveInARow(board, stone, color))
                return true;
        return false;
    }

    // 'X' = pierre de color (pos compris), '_' = vide, 'O' = adverse ou hors du plateau. pos est à l'index 5.
    std::string lineAround(const Board& board, Pos pos, Pos dir, Cell color) {
        std::string line;
        for (int i = -5; i <= 5; i++) {
            const Pos cur = step(pos, dir, i);
            if (i == 0 || (board.isInside(cur) && board.at(cur) == color))
                line += 'X';
            else if (board.isInside(cur) && board.at(cur) == Cell::Empty)
                line += '_';
            else
                line += 'O';
        }
        return line;
    }

    bool hasFreeThree(const Board& board, Pos pos, Pos dir, Cell color) {
        constexpr size_t POS_INDEX = 5;
        const std::string line = lineAround(board, pos, dir, color);
        for (const std::string_view pattern : {"_XXX_", "_XX_X_", "_X_XX_"}) {
            for (size_t start = line.find(pattern); start != std::string::npos; start = line.find(pattern, start + 1))
                if (start <= POS_INDEX && POS_INDEX < start + pattern.size())
                    return true;
        }
        return false;
    }

}

namespace Rules {

std::vector<Pos> findCaptures(const Board& board, Pos pos, Cell color) {
    assert(board.isInside(pos) && "findCaptures() hors du plateau");
    assert((board.at(pos) == color || board.at(pos) == Cell::Empty)
           && "findCaptures() : la case doit être vide ou contenir la pierre de color");

    std::vector<Pos> captured;
    for (const Pos& dir : DIRECTIONS) {
        const Pos opposite = {-dir.x, -dir.y};
        for (const Pos& d : {dir, opposite}) {
            if (checkPatternCapture(board, pos, d, color)) {
                captured.push_back(step(pos, d, 1));
                captured.push_back(step(pos, d, 2));
            }
        }
    }
    return captured;
}

bool makesFiveInARow(const Board& board, Pos pos, Cell color) {
    assert(board.isInside(pos) && "makesFiveInARow() hors du plateau");
    assert(board.at(pos) == color && "makesFiveInARow() s'appelle après avoir posé la pierre de color en pos");

    for (const Pos& dir : DIRECTIONS)
    {
        const Pos opposite = {-dir.x, -dir.y};
        const int total = 1
                        + countInDirection(board, pos, dir, color)
                        + countInDirection(board, pos, opposite, color);
        if (total >= 5)
            return true;
    }
    return false;
}

bool hasFiveInARow(const Board& board, Cell color) {
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            if (board.at({x, y}) == color && makesFiveInARow(board, {x, y}, color))
                return true;
    return false;
}

bool canAnswerFive(const Board& board, Pos fivePos, Cell fiveColor) {
    assert(board.isInside(fivePos) && "canAnswerFive() hors du plateau");
    assert(board.at(fivePos) == fiveColor && "canAnswerFive() attend une pierre du 5 en fivePos");
    const Cell answerer = opponent(fiveColor);
    const std::vector<Pos> fiveStones = stonesOfFives(board, fivePos, fiveColor);

    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            const Pos pos = {x, y};
            if (board.at(pos) != Cell::Empty)
                continue;
            const std::vector<Pos> captured = findCaptures(board, pos, answerer);
            if (captured.empty())
                continue;

            Board after = board;
            after.place(pos, answerer);
            for (const Pos& stone : captured)
                after.capture(stone);
            if (after.captureCount(answerer) >= WINNING_CAPTURES || !fiveSurvives(after, fiveStones, fiveColor))
                return true;
        }
    }
    return false;
}

bool createsDoubleFreeThree(const Board& board, Pos pos, Cell color) {
    assert(board.isInside(pos) && "createsDoubleFreeThree() hors du plateau");
    assert(color != Cell::Empty && "createsDoubleFreeThree() demande la couleur du joueur");
    assert(board.at(pos) == Cell::Empty && "createsDoubleFreeThree() s'appelle avant de poser la pierre");
    if (!findCaptures(board, pos, color).empty())
        return false;

    int freeThrees = 0;
    for (const Pos& dir : DIRECTIONS)
        if (hasFreeThree(board, pos, dir, color))
            freeThrees++;
    return freeThrees >= 2;
}

}  // namespace Rules
