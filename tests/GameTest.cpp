#include "core/Game.hpp"

#include <initializer_list>

#include "support/TestRunner.hpp"

namespace {

    // Les coups alternent Noir, Blanc, Noir... à partir du joueur courant.
    void playAll(Game& game, std::initializer_list<Pos> moves) {
        for (const Pos& move : moves)
            CHECK(game.play(move).isLegal);
    }

    // Noir capture une paire blanche sur la ligne y, Blanc finit par un coup loin à droite : c'est de nouveau à Noir.
    void blackCapturesOnRow(Game& game, int y) {
        playAll(game, {{0, y}, {1, y}, {10, y}, {2, y}, {3, y}, {15, y}});
    }

    // Noir aligne 5 sur la ligne 2 (le dernier coup est en {1, 2}), mais la paire {1, 1}-{1, 2}
    // est encadrée par Blanc en {1, 0} : Blanc peut la casser en jouant {1, 3}.
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

    void testNewGame() {
        const Game game;
        CHECK(game.currentPlayer() == Cell::Black);
        CHECK(!game.winner());
        CHECK(game.captureCount(Cell::Black) == 0);
        CHECK(game.captureCount(Cell::White) == 0);
    }

    void testPlayAlternatesPlayers() {
        Game game;
        CHECK(game.play({9, 9}).isLegal);
        CHECK(game.board().at({9, 9}) == Cell::Black);
        CHECK(game.currentPlayer() == Cell::White);
        CHECK(game.play({10, 10}).isLegal);
        CHECK(game.board().at({10, 10}) == Cell::White);
        CHECK(game.currentPlayer() == Cell::Black);
    }

    // ---- Coups refusés ----

    void testOccupiedCellIsRejected() {
        Game game;
        playAll(game, {{9, 9}});
        const MoveResult result = game.play({9, 9});
        CHECK(!result.isLegal);
        CHECK(result.rejectionReason == "Position is already occupied");
        CHECK(game.currentPlayer() == Cell::White);
    }

    void testOutsideCellIsRejected() {
        Game game;
        for (const Pos& pos : {Pos{-1, 0}, Pos{0, -1}, Pos{BOARD_SIZE, 0}, Pos{0, BOARD_SIZE}}) {
            const MoveResult result = game.play(pos);
            CHECK(!result.isLegal);
            CHECK(result.rejectionReason == "Position is outside the board");
        }
        CHECK(game.currentPlayer() == Cell::Black);
    }

    // Même position que l'exemple a de l'annexe du sujet.
    void testDoubleThreeIsRejected() {
        Game game;
        playAll(game, {{2, 2}, {15, 0}, {3, 3}, {15, 2}, {6, 5}, {15, 4}, {7, 5}, {15, 6}});
        const MoveResult result = game.play({5, 5});
        CHECK(!result.isLegal);
        CHECK(result.rejectionReason == "Double free-three is forbidden");
        CHECK(game.board().at({5, 5}) == Cell::Empty);
        CHECK(!game.isLegalMove({5, 5}));
    }

    void testRejectedMoveIsNotInHistory() {
        Game game;
        playAll(game, {{9, 9}});
        CHECK(!game.play({9, 9}).isLegal);
        game.undo();
        CHECK(game.board().at({9, 9}) == Cell::Empty);
        CHECK(game.currentPlayer() == Cell::Black);
    }

    // ---- Captures ----

    void testCaptureRemovesThePair() {
        Game game;
        playAll(game, {{0, 0}, {1, 0}, {10, 10}, {2, 0}});
        const MoveResult result = game.play({3, 0});
        CHECK(result.isLegal);
        CHECK(result.captured.size() == 2);
        CHECK(game.board().at({1, 0}) == Cell::Empty);
        CHECK(game.board().at({2, 0}) == Cell::Empty);
        CHECK(game.captureCount(Cell::Black) == 2);
        CHECK(game.captureCount(Cell::White) == 0);
    }

    // Annexe du sujet : Rouge (Blanc ici) peut jouer en a entre deux pierres adverses sans perdre sa paire.
    void testMovingIntoACaptureIsSafe() {
        Game game;
        playAll(game, {{0, 0}, {1, 0}, {3, 0}});
        const MoveResult result = game.play({2, 0});
        CHECK(result.isLegal);
        CHECK(result.captured.empty());
        CHECK(game.board().at({1, 0}) == Cell::White);
        CHECK(game.board().at({2, 0}) == Cell::White);
    }

    void testCapturedCellCanBePlayedAgain() {
        Game game;
        playAll(game, {{0, 0}, {1, 0}, {10, 10}, {2, 0}, {3, 0}});
        CHECK(game.play({1, 0}).isLegal);
        CHECK(game.board().at({1, 0}) == Cell::White);
    }

    // ---- Fin de partie ----

    void testTenCapturesWin() {
        Game game;
        for (int i = 0; i < 4; i++)
            blackCapturesOnRow(game, i * 2);
        CHECK(game.captureCount(Cell::Black) == 8);
        CHECK(!game.winner());
        playAll(game, {{0, 8}, {1, 8}, {10, 8}, {2, 8}, {3, 8}});
        CHECK(game.captureCount(Cell::Black) == 10);
        CHECK(game.winner() == Cell::Black);
    }

    void testUnbreakableFiveWins() {
        Game game;
        playAll(game, {{0, 0}, {0, 10}, {1, 0}, {1, 10}, {2, 0}, {2, 10}, {3, 0}, {3, 10}, {4, 0}});
        CHECK(game.winner() == Cell::Black);
    }

    void testNoMoveAfterTheGameIsOver() {
        Game game;
        playAll(game, {{0, 0}, {0, 10}, {1, 0}, {1, 10}, {2, 0}, {2, 10}, {3, 0}, {3, 10}, {4, 0}});
        const MoveResult result = game.play({9, 9});
        CHECK(!result.isLegal);
        CHECK(result.rejectionReason == "The game is over");
    }

    void testBreakableFiveDoesNotWinYet() {
        Game game;
        blackMakesBreakableFive(game);
        CHECK(!game.winner());
    }

    void testFiveNotBrokenWinsOnTheNextMove() {
        Game game;
        blackMakesBreakableFive(game);
        playAll(game, {{15, 15}});
        CHECK(game.winner() == Cell::Black);
    }

    void testBrokenFiveDoesNotWin() {
        Game game;
        blackMakesBreakableFive(game);
        playAll(game, {{1, 3}});
        CHECK(game.captureCount(Cell::White) == 2);
        CHECK(!game.winner());
        playAll(game, {{17, 17}, {17, 15}});
        CHECK(!game.winner());
    }

    // Blanc a un 5 en sursis parce que Noir, à 8 captures, peut prendre une paire n'importe où.
    // Noir la prend loin du 5 : il atteint 10 et gagne, même si le 5 blanc est toujours là.
    void testTenCapturesBeatAPendingFive() {
        Game game;
        for (int i = 0; i < 4; i++)
            blackCapturesOnRow(game, i * 2);
        playAll(game, {
            {0, 14}, {1, 14},
            {18, 18}, {2, 14},
            {18, 16}, {5, 12},
            {18, 14}, {6, 12},
            {18, 12}, {7, 12},
            {18, 10}, {8, 12},
            {18, 8}, {9, 12},
        });
        CHECK(!game.winner());
        playAll(game, {{3, 14}});
        CHECK(game.captureCount(Cell::Black) == 10);
        CHECK(game.winner() == Cell::Black);
    }

    // ---- Undo et reset ----

    void testUndoRestoresTheBoardAndThePlayer() {
        Game game;
        playAll(game, {{0, 0}, {1, 0}, {10, 10}, {2, 0}, {3, 0}});
        game.undo();
        CHECK(game.board().at({3, 0}) == Cell::Empty);
        CHECK(game.board().at({1, 0}) == Cell::White);
        CHECK(game.board().at({2, 0}) == Cell::White);
        CHECK(game.captureCount(Cell::Black) == 0);
        CHECK(game.currentPlayer() == Cell::Black);
    }

    void testUndoRemovesTheWinner() {
        Game game;
        playAll(game, {{0, 0}, {0, 10}, {1, 0}, {1, 10}, {2, 0}, {2, 10}, {3, 0}, {3, 10}, {4, 0}});
        game.undo();
        CHECK(!game.winner());
        CHECK(game.currentPlayer() == Cell::Black);
        CHECK(game.play({4, 0}).isLegal);
        CHECK(game.winner() == Cell::Black);
    }

    // Si undo oubliait le 5 en sursis, Blanc pourrait jouer ailleurs sans perdre.
    void testUndoRestoresThePendingFive() {
        Game game;
        blackMakesBreakableFive(game);
        playAll(game, {{1, 3}});
        game.undo();
        CHECK(game.board().at({1, 1}) == Cell::Black);
        CHECK(game.board().at({1, 2}) == Cell::Black);
        playAll(game, {{15, 15}});
        CHECK(game.winner() == Cell::Black);
    }

    void testUndoAllMovesReturnsToTheStart() {
        Game game;
        blackMakesBreakableFive(game);
        playAll(game, {{1, 3}});
        for (int i = 0; i < 12; i++)
            game.undo();
        for (int y = 0; y < BOARD_SIZE; y++)
            for (int x = 0; x < BOARD_SIZE; x++)
                CHECK(game.board().at({x, y}) == Cell::Empty);
        CHECK(game.currentPlayer() == Cell::Black);
        CHECK(game.captureCount(Cell::White) == 0);
    }

    void testResetStartsANewGame() {
        Game game;
        blackMakesBreakableFive(game);
        game.reset();
        CHECK(game.board().at({1, 2}) == Cell::Empty);
        CHECK(game.currentPlayer() == Cell::Black);
        CHECK(!game.winner());
        playAll(game, {{15, 15}, {16, 16}});
        CHECK(!game.winner());
    }
}

int main() {
    TestRunner::printTitle("Tests de Game");

    TestRunner::run("partie : état initial", testNewGame);
    TestRunner::run("partie : les joueurs alternent", testPlayAlternatesPlayers);

    TestRunner::run("refus : case occupée", testOccupiedCellIsRejected);
    TestRunner::run("refus : hors du plateau", testOutsideCellIsRejected);
    TestRunner::run("refus : double-trois (annexe)", testDoubleThreeIsRejected);
    TestRunner::run("refus : un coup refusé n'entre pas dans l'historique", testRejectedMoveIsNotInHistory);

    TestRunner::run("capture : la paire disparaît", testCaptureRemovesThePair);
    TestRunner::run("capture : jouer entre deux pierres adverses (annexe)", testMovingIntoACaptureIsSafe);
    TestRunner::run("capture : la case libérée se rejoue", testCapturedCellCanBePlayedAgain);

    TestRunner::run("fin : 10 captures gagnent", testTenCapturesWin);
    TestRunner::run("fin : un 5 non cassable gagne", testUnbreakableFiveWins);
    TestRunner::run("fin : plus de coup après la victoire", testNoMoveAfterTheGameIsOver);
    TestRunner::run("fin : un 5 cassable ne gagne pas tout de suite", testBreakableFiveDoesNotWinYet);
    TestRunner::run("fin : un 5 non cassé gagne au coup suivant", testFiveNotBrokenWinsOnTheNextMove);
    TestRunner::run("fin : un 5 cassé ne gagne pas", testBrokenFiveDoesNotWin);
    TestRunner::run("fin : 10 captures battent un 5 en sursis", testTenCapturesBeatAPendingFive);

    TestRunner::run("undo : plateau et joueur restaurés", testUndoRestoresTheBoardAndThePlayer);
    TestRunner::run("undo : le vainqueur disparaît", testUndoRemovesTheWinner);
    TestRunner::run("undo : le 5 en sursis revient", testUndoRestoresThePendingFive);
    TestRunner::run("undo : tout annuler revient au départ", testUndoAllMovesReturnsToTheStart);
    TestRunner::run("reset : nouvelle partie", testResetStartsANewGame);

    return TestRunner::finish();
}
