#include "app/AIDebug.hpp"
#include "core/ai/Eval.hpp"
#include "core/ai/Search.hpp"

#include <initializer_list>
#include <random>
#include <string>

#include "support/TestRunner.hpp"

namespace {

    constexpr int    TEST_DEPTH = 2;
    constexpr double TEST_TIME_LIMIT_MS = 450.0;
    constexpr int    SELF_PLAY_MOVES = 30;

    // Les coups alternent Noir, Blanc, Noir... à partir du joueur courant.
    void playAll(Game& game, std::initializer_list<Pos> moves) {
        for (const Pos& move : moves)
            CHECK(game.play(move).isLegal);
    }

    Pos bestMove(Game& game) {
        Search search;
        return search.findBestMove(game, TEST_DEPTH, TEST_TIME_LIMIT_MS).pos;
    }

    bool samePos(Pos a, Pos b) {
        return a.x == b.x && a.y == b.y;
    }

    // Noir capture une paire blanche sur la ligne y, Blanc finit par un coup loin à droite : c'est de nouveau à Noir.
    void blackCapturesOnRow(Game& game, int y) {
        playAll(game, {{0, y}, {1, y}, {10, y}, {2, y}, {3, y}, {15, y}});
    }

    // Noir aligne 5 sur la ligne 2, mais Blanc peut casser la paire {1, 1}-{1, 2} en jouant {1, 3}.
    void blackMakesBreakableFive(Game& game) {
        playAll(game, {
            {1, 1}, {1, 0},
            {0, 2}, {10, 10},
            {2, 2}, {10, 12},
            {3, 2}, {12, 10},
            {4, 2}, {12, 12},
            {1, 2},
        });
    }

    bool checkEvalConsistent(const Game& game) {
        const bool consistent = Eval::evaluate(game, Cell::Black) == Eval::evaluateFromScratch(game, Cell::Black);
        CHECK(consistent);
        return consistent;
    }

    // Le score incrémental de Board doit suivre chaque coup (captures comprises) et chaque undo.
    void testIncrementalEvalMatchesFromScratch() {
        std::mt19937 rng(7);
        for (int round = 0; round < 20; round++) {
            Game game;
            int played = 0;
            for (int i = 0; i < 400 && !game.winner(); i++) {
                const Pos pos = {static_cast<int>(rng() % 9) + 5, static_cast<int>(rng() % 9) + 5};
                if (!game.isLegalMove(pos))
                    continue;
                game.playLegal(pos);
                played++;
                if (!checkEvalConsistent(game))
                    return;
            }
            for (int i = 0; i < played; i++) {
                game.undo();
                if (!checkEvalConsistent(game))
                    return;
            }
        }
    }

    void testEmptyBoardIsWorthZero() {
        const Game game;
        CHECK(Eval::evaluate(game, Cell::Black) == 0);
    }

    void testScoreDependsOnThePointOfView() {
        Game game;
        playAll(game, {{5, 5}, {0, 0}, {6, 5}, {18, 0}, {7, 5}});
        CHECK(Eval::evaluate(game, Cell::Black) > 0);
        CHECK(Eval::evaluate(game, Cell::Black) == -Eval::evaluate(game, Cell::White));
    }

    void testOpenThreeCountsOnce() {
        Game game;
        playAll(game, {{5, 5}, {0, 0}, {6, 5}, {18, 0}, {7, 5}});
        CHECK(Eval::evaluate(game, Cell::Black) == 200);
    }

    void testThreeBlockedOnOneSideIsWorthHalf() {
        Game game;
        playAll(game, {{5, 5}, {4, 5}, {6, 5}, {18, 0}, {7, 5}});
        CHECK(Eval::evaluate(game, Cell::Black) == 100);
    }

    void testThreeAgainstTheEdgeIsWorthHalf() {
        Game game;
        playAll(game, {{0, 5}, {18, 0}, {1, 5}, {18, 18}, {2, 5}});
        CHECK(Eval::evaluate(game, Cell::Black) == 100);
    }

    void testThreeBlockedOnBothSidesIsWorthZero() {
        Game game;
        playAll(game, {{5, 5}, {4, 5}, {6, 5}, {8, 5}, {7, 5}});
        CHECK(Eval::evaluate(game, Cell::Black) == 0);
    }

    void testCapturesRaiseTheScore() {
        Game game;
        blackCapturesOnRow(game, 0);
        CHECK(game.captureCount(Cell::Black) == 2);
        CHECK(Eval::evaluate(game, Cell::Black) == 100);
    }

    void testPlaysTheCenterOnAnEmptyBoard() {
        Game game;
        CHECK(samePos(bestMove(game), {BOARD_SIZE / 2, BOARD_SIZE / 2}));
    }

    void testCompletesItsFive() {
        Game game;
        playAll(game, {{5, 5}, {0, 0}, {6, 5}, {18, 0}, {7, 5}, {0, 18}, {8, 5}, {18, 18}});
        CHECK(game.play(bestMove(game)).isLegal);
        CHECK(game.winner() == Cell::Black);
    }

    void testBlocksAFourClosedOnOneSide() {
        Game game;
        playAll(game, {{9, 9}, {8, 9}, {10, 9}, {0, 0}, {11, 9}, {18, 0}, {12, 9}});
        CHECK(samePos(bestMove(game), {13, 9}));
    }

    void testPlaysTheTenthCapture() {
        Game game;
        for (int i = 0; i < 4; i++)
            blackCapturesOnRow(game, i * 2);
        playAll(game, {{0, 8}, {1, 8}, {10, 8}, {2, 8}});
        CHECK(samePos(bestMove(game), {3, 8}));
    }

    void testBreaksAPendingFive() {
        Game game;
        blackMakesBreakableFive(game);
        CHECK(samePos(bestMove(game), {1, 3}));
    }

    void testSearchRestoresTheGame() {
        Game game;
        playAll(game, {{9, 9}, {10, 9}, {8, 8}, {11, 9}, {7, 7}});
        const uint64_t hash = game.board().hash();
        const Cell player = game.currentPlayer();

        bestMove(game);

        CHECK(game.board().hash() == hash);
        CHECK(game.currentPlayer() == player);
        CHECK(game.captureCount(Cell::Black) == 0);
        CHECK(game.captureCount(Cell::White) == 0);
        CHECK(!game.winner());
    }

    void testSearchKeepsThePendingFive() {
        Game game;
        blackMakesBreakableFive(game);

        bestMove(game);

        playAll(game, {{15, 15}});
        CHECK(game.winner() == Cell::Black);
    }

    void testSelfPlayOnlyPlaysLegalMoves() {
        Game game;
        Search search;
        for (int i = 0; i < SELF_PLAY_MOVES && !game.winner(); i++) {
            const Pos move = search.findBestMove(game, TEST_DEPTH, TEST_TIME_LIMIT_MS).pos;
            CHECK(game.play(move).isLegal);
        }
    }

    void testReportsTheSearchStats() {
        Game game;
        playAll(game, {{9, 9}, {10, 10}});
        Search search;
        const MoveSuggestion suggestion = search.findBestMove(game, TEST_DEPTH, TEST_TIME_LIMIT_MS);
        CHECK(suggestion.depth == TEST_DEPTH);
        CHECK(suggestion.nodesEvaluated > 0);
    }

    // Débogage : le coup choisi est en tête des coups étudiés, avec le score annoncé, et la liste est triée.
    void testReportsTheRootMoves() {
        Game game;
        playAll(game, {{9, 9}, {10, 10}, {9, 10}});
        Search search;
        const MoveSuggestion suggestion = search.findBestMove(game, TEST_DEPTH, TEST_TIME_LIMIT_MS);
        CHECK(suggestion.rootMoves.size() > 1);
        CHECK(samePos(suggestion.rootMoves[0].pos, suggestion.pos));
        CHECK(suggestion.rootMoves[0].exact);
        CHECK(suggestion.rootMoves[0].score == suggestion.score);
        for (size_t i = 1; i < suggestion.rootMoves.size(); i++)
            CHECK(suggestion.rootMoves[i - 1].score >= suggestion.rootMoves[i].score);
    }

    void testDescribesTheReasoning() {
        Game game;
        playAll(game, {{9, 9}, {10, 10}, {9, 10}});
        Search search;
        const MoveSuggestion suggestion = search.findBestMove(game, TEST_DEPTH, TEST_TIME_LIMIT_MS);
        const std::string text = AIDebug::describe(suggestion);
        const std::string chosen = "coup (" + std::to_string(suggestion.pos.x) + ", " + std::to_string(suggestion.pos.y) + ")";
        CHECK(text.find(chosen) != std::string::npos);
        CHECK(text.find("ligne prévue") != std::string::npos);
        CHECK(text.find("coups étudiés") != std::string::npos);
    }
}

int main() {
    TestRunner::printTitle("Tests de Search et Eval");

    TestRunner::run("eval : plateau vide", testEmptyBoardIsWorthZero);
    TestRunner::run("eval : le signe suit le joueur", testScoreDependsOnThePointOfView);
    TestRunner::run("eval : un trois ouvert compté une fois", testOpenThreeCountsOnce);
    TestRunner::run("eval : un trois bloqué d'un côté", testThreeBlockedOnOneSideIsWorthHalf);
    TestRunner::run("eval : un trois contre le bord", testThreeAgainstTheEdgeIsWorthHalf);
    TestRunner::run("eval : un trois bloqué des deux côtés", testThreeBlockedOnBothSidesIsWorthZero);
    TestRunner::run("eval : les captures comptent", testCapturesRaiseTheScore);
    TestRunner::run("eval : l'incrémental égale le recalcul complet", testIncrementalEvalMatchesFromScratch);

    TestRunner::run("search : centre sur plateau vide", testPlaysTheCenterOnAnEmptyBoard);
    TestRunner::run("search : termine son 5", testCompletesItsFive);
    TestRunner::run("search : bloque un quatre fermé", testBlocksAFourClosedOnOneSide);
    TestRunner::run("search : joue la 10e capture", testPlaysTheTenthCapture);
    TestRunner::run("search : casse un 5 en sursis", testBreaksAPendingFive);
    TestRunner::run("search : rend la partie intacte", testSearchRestoresTheGame);
    TestRunner::run("search : garde le 5 en sursis", testSearchKeepsThePendingFive);
    TestRunner::run("search : que des coups légaux", testSelfPlayOnlyPlaysLegalMoves);
    TestRunner::run("search : statistiques remplies", testReportsTheSearchStats);
    TestRunner::run("debug : coups étudiés à la racine", testReportsTheRootMoves);
    TestRunner::run("debug : raisonnement en texte", testDescribesTheReasoning);

    return TestRunner::finish();
}
