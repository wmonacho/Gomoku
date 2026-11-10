#include "core/Rules.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <string_view>

namespace {

    bool contains(const std::vector<Pos>& stones, Pos pos) {
        return std::find(stones.begin(), stones.end(), pos) != stones.end();
    }

    // Les pierres de removed comptent comme vides : on simule une capture sans copier le plateau.
    int countInDirection(const Board& board, Pos pos, Pos dir, Cell color, const std::vector<Pos>& removed = {}) {
        int count = 0;
        Pos cur = step(pos, dir);
        while (board.isInside(cur) && board.at(cur) == color && !contains(removed, cur)) {
            ++count;
            cur = step(cur, dir);
        }
        return count;
    }

    // color joue en pos : capture dans la direction dir si on a pos, adverse, adverse, color.
    bool checkPatternCapture(const Board& board, Pos pos, Pos dir, Cell color) {
        const Cell enemy = opponent(color);
        const Pos pos1 = step(pos, dir, 1);
        const Pos pos2 = step(pos, dir, 2);
        const Pos pos3 = step(pos, dir, 3);
        return board.isInside(pos3)
            && board.at(pos1) == enemy && board.at(pos2) == enemy && board.at(pos3) == color;
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
    bool fiveSurvives(const Board& board, const std::vector<Pos>& stones, Cell color, const std::vector<Pos>& captured) {
        for (const Pos& stone : stones) {
            if (contains(captured, stone))
                continue;
            for (const Pos& dir : DIRECTIONS) {
                const Pos opposite = {-dir.x, -dir.y};
                const int total = 1
                                + countInDirection(board, stone, dir, color, captured)
                                + countInDirection(board, stone, opposite, color, captured);
                if (total >= 5)
                    return true;
            }
        }
        return false;
    }

    // Dans _XXX_, _XX_X_ et _X_XX_, pos est l'un des 'X' : les deux autres sont à 3 cases ou moins de lui.
    constexpr int FREE_THREE_REACH = 3;

    int stonesNear(const Board& board, Pos pos, Pos dir, Cell color) {
        int count = 0;
        for (int i = -FREE_THREE_REACH; i <= FREE_THREE_REACH; i++) {
            const Pos cur = step(pos, dir, i);
            if (i != 0 && board.isInside(cur) && board.at(cur) == color)
                count++;
        }
        return count;
    }

    // Condition nécessaire, bien moins chère que hasFreeThree : un trois libre demande 2 pierres proches
    // sur sa ligne, un double trois demande donc 2 directions comme ça.
    bool mayCreateDoubleFreeThree(const Board& board, Pos pos, Cell color) {
        int directions = 0;
        for (const Pos& dir : DIRECTIONS)
            if (stonesNear(board, pos, dir, color) >= 2)
                directions++;
        return directions >= 2;
    }

    constexpr int LINE_RADIUS = 5;
    using Line = std::array<char, 2 * LINE_RADIUS + 1>;

    // 'X' = pierre de color (pos compris), '_' = vide, 'O' = adverse ou hors du plateau. pos est au centre.
    Line lineAround(const Board& board, Pos pos, Pos dir, Cell color) {
        Line line{};
        for (int i = -LINE_RADIUS; i <= LINE_RADIUS; i++) {
            const Pos cur = step(pos, dir, i);
            char& c = line[i + LINE_RADIUS];
            if (i == 0 || (board.isInside(cur) && board.at(cur) == color))
                c = 'X';
            else if (board.isInside(cur) && board.at(cur) == Cell::Empty)
                c = '_';
            else
                c = 'O';
        }
        return line;
    }

    bool hasFreeThree(const Board& board, Pos pos, Pos dir, Cell color) {
        constexpr size_t POS_INDEX = LINE_RADIUS;
        const Line chars = lineAround(board, pos, dir, color);
        const std::string_view line(chars.data(), chars.size());
        for (const std::string_view pattern : {"_XXX_", "_XX_X_", "_X_XX_"}) {
            for (size_t start = line.find(pattern); start != std::string_view::npos; start = line.find(pattern, start + 1))
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

// Même test que findCaptures, sans construire la liste : pour les appels très fréquents.
bool canCapture(const Board& board, Pos pos, Cell color) {
    for (const Pos& dir : DIRECTIONS) {
        const Pos opposite = {-dir.x, -dir.y};
        for (const Pos& d : {dir, opposite})
            if (checkPatternCapture(board, pos, d, color))
                return true;
    }
    return false;
}

bool makesFiveInARow(const Board& board, Pos pos, Cell color) {
    assert(board.isInside(pos) && "makesFiveInARow() hors du plateau");
    assert(board.at(pos) == color && "makesFiveInARow() s'appelle après avoir posé la pierre de color en pos");

    for (const Pos& dir : DIRECTIONS) {
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

            const int capturesAfter = board.captureCount(answerer) + static_cast<int>(captured.size());
            if (capturesAfter >= WINNING_CAPTURES || !fiveSurvives(board, fiveStones, fiveColor, captured))
                return true;
        }
    }
    return false;
}

bool createsDoubleFreeThree(const Board& board, Pos pos, Cell color) {
    assert(board.isInside(pos) && "createsDoubleFreeThree() hors du plateau");
    assert(color != Cell::Empty && "createsDoubleFreeThree() demande la couleur du joueur");
    assert(board.at(pos) == Cell::Empty && "createsDoubleFreeThree() s'appelle avant de poser la pierre");
    if (!mayCreateDoubleFreeThree(board, pos, color))
        return false;
    if (canCapture(board, pos, color))
        return false;

    int freeThrees = 0;
    for (const Pos& dir : DIRECTIONS)
        if (hasFreeThree(board, pos, dir, color))
            freeThrees++;
    return freeThrees >= 2;
}

}  // namespace Rules
