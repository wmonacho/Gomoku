#include "support/BoardAscii.hpp"

#include "support/TestRunner.hpp"

namespace {

    // Si l'outil inversait x et y, tous les tests de règles seraient faux : on le vérifie ici.
    void testDrawingMapsToCoordinates() {
        const Board board = boardFrom({
            "...",
            "..X",
            "...",
            "O..",
        });
        CHECK(board.at({2, 1}) == Cell::Black);
        CHECK(board.at({0, 3}) == Cell::White);
        CHECK(board.at({1, 2}) == Cell::Empty);
        CHECK(board.at({1, 0}) == Cell::Empty);
    }

    void testUnderscoreAndDotAreEmpty() {
        const Board board = boardFrom({"_X_.O"});
        CHECK(board.at({0, 0}) == Cell::Empty);
        CHECK(board.at({1, 0}) == Cell::Black);
        CHECK(board.at({2, 0}) == Cell::Empty);
        CHECK(board.at({3, 0}) == Cell::Empty);
        CHECK(board.at({4, 0}) == Cell::White);
    }

    void testCellsOutsideDrawingStayEmpty() {
        const Board board = boardFrom({"XO"});
        for (int y = 0; y < BOARD_SIZE; y++)
            for (int x = 0; x < BOARD_SIZE; x++)
                if (y != 0 || x > 1)
                    CHECK(board.at({x, y}) == Cell::Empty);
    }

    void testFullSizeDrawingReachesLastCell() {
        const Board board = boardFrom({
            "X..................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "...................",
            "..................O",
        });
        CHECK(board.at({0, 0}) == Cell::Black);
        CHECK(board.at({BOARD_SIZE - 1, BOARD_SIZE - 1}) == Cell::White);
    }

    // Aller-retour : dessin -> Board -> lignes doit redonner le dessin.
    void testToRowsGivesBackTheDrawing() {
        const Board board = boardFrom({
            ".XOO",
            "....",
            "O..X",
        });
        const std::vector<std::string> rows = toRows(board);
        CHECK(rows.size() == BOARD_SIZE);
        CHECK(rows[0] == ".XOO...............");
        CHECK(rows[1] == "...................");
        CHECK(rows[2] == "O..X...............");
        CHECK(rows[18] == "...................");
    }

    void testToAsciiShowsCoordinatesAndCaptures() {
        Board board = boardFrom({".XOO"});
        board.place({4, 0}, Cell::Black);
        board.capture({2, 0});
        board.capture({3, 0});

        const std::string text = toAscii(board);
        CHECK(text.find("  0  1  2") != std::string::npos);
        CHECK(text.find(" 18") != std::string::npos);
        CHECK(text.find("  0   .  X  .  .  X  .") != std::string::npos);
        CHECK(text.find("Captures : X = 2, O = 0") != std::string::npos);
    }
}

int main() {
    TestRunner::printTitle("Tests de l'outil BoardAscii");

    TestRunner::run("le dessin donne les bonnes coordonnées", testDrawingMapsToCoordinates);
    TestRunner::run("'_' et '.' sont des cases vides", testUnderscoreAndDotAreEmpty);
    TestRunner::run("les cases hors du dessin restent vides", testCellsOutsideDrawingStayEmpty);
    TestRunner::run("un dessin 19x19 atteint la dernière case", testFullSizeDrawingReachesLastCell);
    TestRunner::run("toRows redonne le dessin", testToRowsGivesBackTheDrawing);
    TestRunner::run("toAscii affiche coordonnées et captures", testToAsciiShowsCoordinatesAndCaptures);

    return TestRunner::finish();
}
