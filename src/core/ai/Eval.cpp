#include "core/ai/Eval.hpp"

#include "core/Rules.hpp"

#include <cassert>

namespace {
    constexpr int CAPTURE_WEIGHT = 50;
    // Une paire que je peux prendre au prochain coup : un peu moins de la moitié d'une capture faite (2 × 50).
    // Réglé par matchs IA contre IA : de 15 à 250 le résultat change peu, 40 est au milieu.
    constexpr int CAPTURE_THREAT_WEIGHT = 40;
    // À 8 pierres prises, la prochaine capture gagne : la menace vaut alors autant qu'un quatre ouvert.
    constexpr int WINNING_THREAT_WEIGHT = 4'000;

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

    int threatWeight(const Game& game, Cell color) {
        return game.captureCount(color) >= Rules::WINNING_CAPTURES - 2 ? WINNING_THREAT_WEIGHT : CAPTURE_THREAT_WEIGHT;
    }

    // Recalcul complet des menaces : depuis chaque pierre de color, une paire adverse puis une case vide.
    int countCaptureThreats(const Board& board, Cell color) {
        int threats = 0;
        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                const Pos pos = {x, y};
                if (board.at(pos) != color)
                    continue;
                for (const Pos& dir : DIRECTIONS)
                    for (const Pos& d : {dir, Pos{-dir.x, -dir.y}})
                        if (holds(board, step(pos, d, 1), opponent(color)) && holds(board, step(pos, d, 2), opponent(color))
                            && holds(board, step(pos, d, 3), Cell::Empty))
                            threats++;
            }
        }
        return threats;
    }

    int scoreColor(const Game& game, Cell color) {
        const Board& board = game.board();
        int score = game.captureCount(color) * CAPTURE_WEIGHT;
        score += countCaptureThreats(board, color) * threatWeight(game, color);

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

    // Même calcul que scoreColor, avec les totaux que Board tient à jour.
    int scoreColorIncremental(const Game& game, Cell color) {
        const Board& board = game.board();
        return game.captureCount(color) * CAPTURE_WEIGHT
             + board.captureThreats(color) * threatWeight(game, color)
             + board.runScore(color);
    }
}

namespace Eval {

int evaluate(const Game& game, Cell color) {
    assert(color != Cell::Empty && "evaluate() demande le point de vue d'un joueur, pas Empty");

    return scoreColorIncremental(game, color) - scoreColorIncremental(game, opponent(color));
}

int evaluateFromScratch(const Game& game, Cell color) {
    assert(color != Cell::Empty && "evaluateFromScratch() demande le point de vue d'un joueur, pas Empty");

    return scoreColor(game, color) - scoreColor(game, opponent(color));
}

}
