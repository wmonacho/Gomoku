#include "core/Rules.hpp"

#include "support/BoardAscii.hpp"
#include "support/TestRunner.hpp"

namespace {

    // ---- Alignement de 5 : Rules::makesFiveInARow ----
    // La pierre testée (pos) doit déjà être posée : c'est ce que Game fera après place().

    void testFiveHorizontal() {
        const Board board = boardFrom({
            "",
            ".XXXXX",
        });
        CHECK(Rules::makesFiveInARow(board, {5, 1}, Cell::Black));
    }

    void testFiveVertical() {
        const Board board = boardFrom({
            "...X",
            "...X",
            "...X",
            "...X",
            "...X",
        });
        CHECK(Rules::makesFiveInARow(board, {3, 0}, Cell::Black));
    }

    void testFiveDiagonalDownRight() {
        const Board board = boardFrom({
            "X....",
            ".X...", 
            "..X..",
            "...X.",
            "....X",
        });
        CHECK(Rules::makesFiveInARow(board, {4, 4}, Cell::Black));
    }

    void testFiveDiagonalUpRight() {
        const Board board = boardFrom({
            "....X",
            "...X.",
            "..X..",
            ".X...",
            "X....",
        });
        CHECK(Rules::makesFiveInARow(board, {0, 4}, Cell::Black));
    }

    // Le coup qui complète l'alignement n'est pas forcément au bout : il faut compter des deux côtés.
    void testFiveWithStoneInTheMiddle() {
        const Board board = boardFrom({
            "",
            "",
            "..XXXXX",
        });
        CHECK(Rules::makesFiveInARow(board, {4, 2}, Cell::Black));
        CHECK(Rules::makesFiveInARow(board, {3, 2}, Cell::Black));
    }

    void testFiveForWhite() {
        const Board board = boardFrom({
            "OOOOO",
        });
        CHECK(Rules::makesFiveInARow(board, {2, 0}, Cell::White));
    }

    // Le sujet : « 5 or more » gagne, donc 6 alignés aussi.
    void testSixInARowAlsoWins() {
        const Board board = boardFrom({
            "",
            "XXXXXX",
        });
        CHECK(Rules::makesFiveInARow(board, {0, 1}, Cell::Black));
        CHECK(Rules::makesFiveInARow(board, {3, 1}, Cell::Black));
    }

    void testFourIsNotFive() {
        const Board board = boardFrom({
            "",
            ".XXXX.",
        });
        CHECK(!Rules::makesFiveInARow(board, {1, 1}, Cell::Black));
        CHECK(!Rules::makesFiveInARow(board, {4, 1}, Cell::Black));
    }

    void testOpponentStoneBreaksTheLine() {
        const Board board = boardFrom({
            "XXOXXX",
        });
        CHECK(!Rules::makesFiveInARow(board, {0, 0}, Cell::Black));
        CHECK(!Rules::makesFiveInARow(board, {5, 0}, Cell::Black));
    }

    void testGapBreaksTheLine() {
        const Board board = boardFrom({
            "XX_XX",
        });
        CHECK(!Rules::makesFiveInARow(board, {0, 0}, Cell::Black));
        CHECK(!Rules::makesFiveInARow(board, {4, 0}, Cell::Black));
    }

    // Quatre directions qui ont chacune 4 pierres ne font pas un 5 : on ne cumule pas les directions.
    void testFoursInSeveralDirectionsAreNotFive() {
        const Board board = boardFrom({
            "X..X..X",
            ".X.X.X.",
            "..XXX..",
            "XXXX...",
        });
        CHECK(!Rules::makesFiveInARow(board, {3, 3}, Cell::Black));
    }

    // Un 5 adverse ailleurs sur le plateau ne compte pas pour color.
    void testOpponentFiveDoesNotCount() {
        const Board board = boardFrom({
            "OOOOO",
            "",
            "X",
        });
        CHECK(!Rules::makesFiveInARow(board, {0, 2}, Cell::Black));
    }

    // Bords du plateau : l'alignement touche le bord, et la fonction ne doit pas lire hors du plateau.
    void testFiveAgainstLeftEdge() {
        const Board board = boardFrom({
            "",
            "XXXXX",
        });
        CHECK(Rules::makesFiveInARow(board, {0, 1}, Cell::Black));
    }

    void testFiveAgainstRightEdge() {
        const Board board = boardFrom({
            "",
            "",
            "..............XXXXX",
        });
        CHECK(Rules::makesFiveInARow(board, {BOARD_SIZE - 1, 2}, Cell::Black));
    }

    void testFiveInBottomRightCorner() {
        Board board;
        for (int i = 0; i < 5; i++)
            board.place({BOARD_SIZE - 1 - i, BOARD_SIZE - 1 - i}, Cell::Black);
        CHECK(Rules::makesFiveInARow(board, {BOARD_SIZE - 1, BOARD_SIZE - 1}, Cell::Black));
    }

    void testFourAgainstEdgeIsNotFive() {
        const Board board = boardFrom({
            "XXXX",
        });
        CHECK(!Rules::makesFiveInARow(board, {0, 0}, Cell::Black));
    }

    // ---- Captures : Rules::findCaptures ----
    // Comme pour le 5, la pierre testée (pos) est déjà posée. L'ordre des pierres renvoyées n'est pas imposé.

    bool contains(const std::vector<Pos>& stones, Pos pos) {
        for (const Pos& stone : stones)
            if (stone.x == pos.x && stone.y == pos.y)
                return true;
        return false;
    }

    void testCaptureRight() {
        const Board board = boardFrom({
            "XOOX",
        });
        const std::vector<Pos> captured = Rules::findCaptures(board, {0, 0}, Cell::Black);
        CHECK(captured.size() == 2);
        CHECK(contains(captured, {1, 0}));
        CHECK(contains(captured, {2, 0}));
    }

    // Les 4 sens « négatifs » (gauche, haut, diagonales vers le haut-gauche…) doivent aussi être testés.
    void testCaptureLeft() {
        const Board board = boardFrom({
            "XOOX",
        });
        const std::vector<Pos> captured = Rules::findCaptures(board, {3, 0}, Cell::Black);
        CHECK(captured.size() == 2);
        CHECK(contains(captured, {1, 0}));
        CHECK(contains(captured, {2, 0}));
    }

    void testCaptureUp() {
        const Board board = boardFrom({
            "X",
            "O",
            "O",
            "X",
        });
        const std::vector<Pos> captured = Rules::findCaptures(board, {0, 3}, Cell::Black);
        CHECK(captured.size() == 2);
        CHECK(contains(captured, {0, 1}));
        CHECK(contains(captured, {0, 2}));
    }

    void testCaptureDiagonalUpLeft() {
        const Board board = boardFrom({
            "X...",
            ".O..",
            "..O.",
            "...X",
        });
        const std::vector<Pos> captured = Rules::findCaptures(board, {3, 3}, Cell::Black);
        CHECK(captured.size() == 2);
        CHECK(contains(captured, {1, 1}));
        CHECK(contains(captured, {2, 2}));
    }

    void testCaptureForWhite() {
        const Board board = boardFrom({
            "OXXO",
        });
        const std::vector<Pos> captured = Rules::findCaptures(board, {3, 0}, Cell::White);
        CHECK(captured.size() == 2);
        CHECK(contains(captured, {1, 0}));
        CHECK(contains(captured, {2, 0}));
    }

    // Un seul coup peut capturer dans plusieurs directions : toutes les paires sont renvoyées.
    void testDoubleCapture() {
        const Board board = boardFrom({
            "XOOX",
            "O",
            "O",
            "X",
        });
        const std::vector<Pos> captured = Rules::findCaptures(board, {0, 0}, Cell::Black);
        CHECK(captured.size() == 4);
        CHECK(contains(captured, {1, 0}));
        CHECK(contains(captured, {2, 0}));
        CHECK(contains(captured, {0, 1}));
        CHECK(contains(captured, {0, 2}));
    }

    // Le maximum : une paire dans chacune des 8 directions, soit MAX_CAPTURES_PER_MOVE pierres.
    void testCaptureInAllEightDirections() {
        const Board board = boardFrom({
            "X..X..X",
            ".O.O.O.",
            "..OOO..",
            "XOOXOOX",
            "..OOO..",
            ".O.O.O.",
            "X..X..X",
        });
        const std::vector<Pos> captured = Rules::findCaptures(board, {3, 3}, Cell::Black);
        CHECK(captured.size() == MAX_CAPTURES_PER_MOVE);
    }

    // Seule une paire exacte est capturable.
    void testSingleStoneIsNotCaptured() {
        const Board board = boardFrom({
            "XOX",
        });
        CHECK(Rules::findCaptures(board, {0, 0}, Cell::Black).empty());
    }

    void testThreeStonesAreNotCaptured() {
        const Board board = boardFrom({
            "XOOOX",
        });
        CHECK(Rules::findCaptures(board, {0, 0}, Cell::Black).empty());
    }

    void testOpenPairIsNotCaptured() {
        const Board board = boardFrom({
            "XOO.",
        });
        CHECK(Rules::findCaptures(board, {0, 0}, Cell::Black).empty());
    }

    void testPairClosedByOpponentIsNotCaptured() {
        const Board board = boardFrom({
            "XOOO",
        });
        CHECK(Rules::findCaptures(board, {0, 0}, Cell::Black).empty());
    }

    void testOwnStoneInsideIsNotCaptured() {
        const Board board = boardFrom({
            "XOXX",
            "XXOX",
        });
        CHECK(Rules::findCaptures(board, {0, 0}, Cell::Black).empty());
        CHECK(Rules::findCaptures(board, {0, 1}, Cell::Black).empty());
    }

    // Seule la pierre qui vient d'être posée capture : un motif déjà présent ailleurs ne compte pas.
    void testExistingPatternElsewhereIsIgnored() {
        const Board board = boardFrom({
            "XOOX",
            "",
            "X",
        });
        CHECK(Rules::findCaptures(board, {0, 2}, Cell::Black).empty());
    }

    // Jouer entre deux pierres adverses est autorisé : sa propre paire n'est pas capturée.
    void testNoSelfCapture() {
        const Board board = boardFrom({
            "OXXO",
        });
        CHECK(Rules::findCaptures(board, {1, 0}, Cell::Black).empty());
        CHECK(Rules::findCaptures(board, {2, 0}, Cell::Black).empty());
    }

    // Bords du plateau : le motif touche le bord, ou déborderait du plateau. Rien ne doit être lu dehors.
    void testCaptureClosedAgainstLeftEdge() {
        const Board board = boardFrom({
            "XOOX",
        });
        const std::vector<Pos> captured = Rules::findCaptures(board, {3, 0}, Cell::Black);
        CHECK(captured.size() == 2);
    }

    void testCaptureClosedAgainstRightEdge() {
        const Board board = boardFrom({
            "...............XOOX",
        });
        const std::vector<Pos> captured = Rules::findCaptures(board, {BOARD_SIZE - 4, 0}, Cell::Black);
        CHECK(captured.size() == 2);
        CHECK(contains(captured, {BOARD_SIZE - 3, 0}));
        CHECK(contains(captured, {BOARD_SIZE - 2, 0}));
    }

    // La paire touche le bord : la case qui fermerait le motif est hors du plateau.
    void testPairAgainstRightEdgeIsNotCaptured() {
        const Board board = boardFrom({
            "................XOO",
        });
        CHECK(Rules::findCaptures(board, {BOARD_SIZE - 3, 0}, Cell::Black).empty());
    }

    void testPairAgainstBottomEdgeIsNotCaptured() {
        Board board;
        board.place({0, BOARD_SIZE - 3}, Cell::Black);
        board.place({0, BOARD_SIZE - 2}, Cell::White);
        board.place({0, BOARD_SIZE - 1}, Cell::White);
        CHECK(Rules::findCaptures(board, {0, BOARD_SIZE - 3}, Cell::Black).empty());
    }

    void testPairAgainstCornerDiagonalIsNotCaptured() {
        Board board;
        board.place({BOARD_SIZE - 3, BOARD_SIZE - 3}, Cell::Black);
        board.place({BOARD_SIZE - 2, BOARD_SIZE - 2}, Cell::White);
        board.place({BOARD_SIZE - 1, BOARD_SIZE - 1}, Cell::White);
        CHECK(Rules::findCaptures(board, {BOARD_SIZE - 3, BOARD_SIZE - 3}, Cell::Black).empty());
    }

    void testCaptureInBottomRightCorner() {
        Board board;
        board.place({BOARD_SIZE - 4, BOARD_SIZE - 4}, Cell::Black);
        board.place({BOARD_SIZE - 3, BOARD_SIZE - 3}, Cell::White);
        board.place({BOARD_SIZE - 2, BOARD_SIZE - 2}, Cell::White);
        board.place({BOARD_SIZE - 1, BOARD_SIZE - 1}, Cell::Black);
        const std::vector<Pos> captured = Rules::findCaptures(board, {BOARD_SIZE - 1, BOARD_SIZE - 1}, Cell::Black);
        CHECK(captured.size() == 2);
        CHECK(contains(captured, {BOARD_SIZE - 3, BOARD_SIZE - 3}));
        CHECK(contains(captured, {BOARD_SIZE - 2, BOARD_SIZE - 2}));
    }

    // Une pierre dans chaque coin : les 8 directions partent hors du plateau dès le premier pas pour certaines.
    void testStoneInEachCornerDoesNotCrash() {
        const Pos corners[] = {{0, 0}, {BOARD_SIZE - 1, 0}, {0, BOARD_SIZE - 1}, {BOARD_SIZE - 1, BOARD_SIZE - 1}};
        for (const Pos& corner : corners) {
            Board board;
            board.place(corner, Cell::Black);
            CHECK(Rules::findCaptures(board, corner, Cell::Black).empty());
        }
    }

    // Balayage complet : chaque case du plateau, entourée de pierres adverses jusqu'au bord.
    // Un accès hors du plateau ferait sauter l'assert de Board::at.
    void testEveryCellNextToOpponentsDoesNotCrash() {
        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                Board board;
                for (int oy = 0; oy < BOARD_SIZE; oy++)
                    for (int ox = 0; ox < BOARD_SIZE; ox++)
                        if (ox != x || oy != y)
                            board.place({ox, oy}, Cell::White);
                board.place({x, y}, Cell::Black);
                CHECK(Rules::findCaptures(board, {x, y}, Cell::Black).empty());
            }
        }
    }

    // Avant de poser la pierre : sert à savoir ce qu'un coup capturerait.
    void testCaptureFromEmptyCell() {
        const Board board = boardFrom({
            "XOO.",
        });
        const std::vector<Pos> captured = Rules::findCaptures(board, {3, 0}, Cell::Black);
        CHECK(captured.size() == 2);
        CHECK(contains(captured, {1, 0}));
        CHECK(contains(captured, {2, 0}));
    }

    // Annexe du sujet, VI.1 : Bleu (X) joue en a et capture la paire rouge en diagonale.
    void testSubjectCaptureExample() {
        const Board board = boardFrom({
            "X...",
            ".O..",
            "..O.",
            "...X",
        });
        const std::vector<Pos> captured = Rules::findCaptures(board, {3, 3}, Cell::Black);
        CHECK(captured.size() == 2);
        CHECK(contains(captured, {1, 1}));
        CHECK(contains(captured, {2, 2}));
    }

    // ---- Double-trois : Rules::createsDoubleFreeThree ----
    // La case testée (pos) doit être vide : c'est ce que Game vérifie avant place().

    // Annexe du sujet, VI.2 : le trois en diagonale, complété ici par un trois horizontal sur la même case.
    void testSubjectDiagonalFreeThree() {
        const Board board = boardFrom({
            "",
            ".X",
            "..X",
            ".XX.",
            "",
        });
        CHECK(Rules::createsDoubleFreeThree(board, {3, 3}, Cell::Black));
    }

    // Annexe du sujet, VI.2 : le trois avec un trou (_X_XX_), complété ici par un trois vertical.
    void testSubjectSplitFreeThree() {
        const Board board = boardFrom({
            "",
            "....X",
            "....X",
            ".X.X",
            "",
        });
        CHECK(Rules::createsDoubleFreeThree(board, {4, 3}, Cell::Black));
    }

    // Annexe du sujet, VI.2 : jouer en a crée deux trois libres (diagonale _XX_X_ et horizontale _XXX_).
    void testSubjectDoubleThreeIsForbidden() {
        const Board board = boardFrom({
            "",
            "",
            "..X",
            "...X",
            "",
            "......XX",
        });
        CHECK(Rules::createsDoubleFreeThree(board, {5, 5}, Cell::Black));
    }

    // Même position avec une pierre adverse en b : le trois horizontal est bloqué, a devient légal.
    void testSubjectDoubleThreeBlockedInB() {
        const Board board = boardFrom({
            "",
            "",
            "..X",
            "...X",
            "",
            "....O.XX",
        });
        CHECK(!Rules::createsDoubleFreeThree(board, {5, 5}, Cell::Black));
    }

    void testSingleFreeThreeIsAllowed() {
        const Board board = boardFrom({
            "",
            ".XX.",
        });
        CHECK(!Rules::createsDoubleFreeThree(board, {3, 1}, Cell::Black));
    }

    void testThreeBlockedByOpponentIsNotFree() {
        const Board board = boardFrom({
            "",
            "...X",
            "...X",
            "OXX.",
        });
        CHECK(!Rules::createsDoubleFreeThree(board, {3, 3}, Cell::Black));
    }

    void testDoubleThreeForWhite() {
        const Board board = boardFrom({
            "",
            "...O",
            "...O",
            ".OO.",
            "",
        });
        CHECK(Rules::createsDoubleFreeThree(board, {3, 3}, Cell::White));
        CHECK(!Rules::createsDoubleFreeThree(board, {3, 3}, Cell::Black));
    }

    // Exception du sujet : un double-trois est autorisé s'il est créé en capturant une paire.
    void testDoubleThreeByCaptureIsAllowed() {
        const Board board = boardFrom({
            "",
            "...X",
            "...X",
            ".XX.OOX",
            "",
        });
        CHECK(!Rules::createsDoubleFreeThree(board, {3, 3}, Cell::Black));
    }

    // Le bord bloque comme une pierre adverse.
    void testThreeAgainstTopEdgeIsNotFree() {
        const Board board = boardFrom({
            "...X",
            "...X",
            ".XX.",
            "",
        });
        CHECK(!Rules::createsDoubleFreeThree(board, {3, 2}, Cell::Black));
    }

    void testThreeAgainstLeftEdgeIsNotFree() {
        const Board board = boardFrom({
            "",
            "..X",
            "..X",
            "XX.",
            "",
        });
        CHECK(!Rules::createsDoubleFreeThree(board, {2, 3}, Cell::Black));
    }

    // Une case libre avant le bord suffit : _XXX_ peut encore devenir un quatre ouvert.
    void testDoubleThreeOneCellFromTheCorner() {
        Board board;
        board.place({BOARD_SIZE - 3, BOARD_SIZE - 2}, Cell::Black);
        board.place({BOARD_SIZE - 4, BOARD_SIZE - 2}, Cell::Black);
        board.place({BOARD_SIZE - 2, BOARD_SIZE - 3}, Cell::Black);
        board.place({BOARD_SIZE - 2, BOARD_SIZE - 4}, Cell::Black);
        CHECK(Rules::createsDoubleFreeThree(board, {BOARD_SIZE - 2, BOARD_SIZE - 2}, Cell::Black));
    }

    void testDoubleThreeAgainstTheCornerIsAllowed() {
        Board board;
        board.place({BOARD_SIZE - 2, BOARD_SIZE - 1}, Cell::Black);
        board.place({BOARD_SIZE - 3, BOARD_SIZE - 1}, Cell::Black);
        board.place({BOARD_SIZE - 1, BOARD_SIZE - 2}, Cell::Black);
        board.place({BOARD_SIZE - 1, BOARD_SIZE - 3}, Cell::Black);
        CHECK(!Rules::createsDoubleFreeThree(board, {BOARD_SIZE - 1, BOARD_SIZE - 1}, Cell::Black));
    }

    // ---- Réponse à un 5 : Rules::canAnswerFive ----

    // boardFrom ne règle pas les compteurs : on joue de vraies captures sur la dernière ligne.
    void giveCapturedPairs(Board& board, Cell capturer, int pairs) {
        const int y = BOARD_SIZE - 1;
        for (int i = 0; i < pairs; i++) {
            const int x = i * 4;
            board.place({x, y}, opponent(capturer));
            board.place({x + 1, y}, opponent(capturer));
            board.place({x + 2, y}, capturer);
            board.capture({x, y});
            board.capture({x + 1, y});
        }
    }

    void testFiveWithNothingToCaptureWins() {
        const Board board = boardFrom({
            "XXXXX",
        });
        CHECK(!Rules::canAnswerFive(board, {0, 0}, Cell::Black));
    }

    void testFiveBrokenByCapture() {
        const Board board = boardFrom({
            ".O...",
            ".X...",
            "XXXXX",
        });
        CHECK(Rules::canAnswerFive(board, {0, 2}, Cell::Black));
    }

    void testCapturingTheCheckedStoneBreaksTheFive() {
        const Board board = boardFrom({
            "O....",
            "X....",
            "XXXXX",
        });
        CHECK(Rules::canAnswerFive(board, {4, 2}, Cell::Black));
    }

    void testSixLosingAnEndStoneStillWins() {
        const Board board = boardFrom({
            "O.....",
            "X.....",
            "XXXXXX",
        });
        CHECK(!Rules::canAnswerFive(board, {1, 2}, Cell::Black));
    }

    void testCaptureAwayFromTheFiveDoesNotAnswer() {
        const Board board = boardFrom({
            "XXXXX.....",
            "",
            "......OXX.",
        });
        CHECK(!Rules::canAnswerFive(board, {0, 0}, Cell::Black));
    }

    // Blanc a déjà 8 pierres : n'importe quelle paire l'amène à 10, même loin du 5.
    void testCaptureAwayFromTheFiveWinsAtEightCaptures() {
        Board board = boardFrom({
            "XXXXX.....",
            "",
            "......OXX.",
        });
        giveCapturedPairs(board, Cell::White, 4);
        CHECK(board.captureCount(Cell::White) == 8);
        CHECK(Rules::canAnswerFive(board, {0, 0}, Cell::Black));
    }

    void testEightCapturesWithoutAPairToTakeDoesNotAnswer() {
        Board board = boardFrom({
            "XXXXX",
        });
        giveCapturedPairs(board, Cell::White, 4);
        CHECK(!Rules::canAnswerFive(board, {0, 0}, Cell::Black));
    }

    void testSixCapturesAreNotEnough() {
        Board board = boardFrom({
            "XXXXX.....",
            "",
            "......OXX.",
        });
        giveCapturedPairs(board, Cell::White, 3);
        CHECK(!Rules::canAnswerFive(board, {0, 0}, Cell::Black));
    }
}

int main() {
    TestRunner::printTitle("Tests de Rules");

    TestRunner::run("5 : horizontal", testFiveHorizontal);
    TestRunner::run("5 : vertical", testFiveVertical);
    TestRunner::run("5 : diagonale ↘", testFiveDiagonalDownRight);
    TestRunner::run("5 : diagonale ↗", testFiveDiagonalUpRight);
    TestRunner::run("5 : pierre jouée au milieu", testFiveWithStoneInTheMiddle);
    TestRunner::run("5 : marche aussi pour Blanc", testFiveForWhite);
    TestRunner::run("5 : 6 alignés gagnent aussi", testSixInARowAlsoWins);
    TestRunner::run("5 : 4 alignés ne suffisent pas", testFourIsNotFive);
    TestRunner::run("5 : une pierre adverse coupe la ligne", testOpponentStoneBreaksTheLine);
    TestRunner::run("5 : un trou coupe la ligne", testGapBreaksTheLine);
    TestRunner::run("5 : plusieurs 4 ne font pas un 5", testFoursInSeveralDirectionsAreNotFive);
    TestRunner::run("5 : le 5 adverse ne compte pas", testOpponentFiveDoesNotCount);
    TestRunner::run("5 : contre le bord gauche", testFiveAgainstLeftEdge);
    TestRunner::run("5 : contre le bord droit", testFiveAgainstRightEdge);
    TestRunner::run("5 : dans le coin bas-droit", testFiveInBottomRightCorner);
    TestRunner::run("5 : 4 contre le bord ne suffisent pas", testFourAgainstEdgeIsNotFive);

    TestRunner::run("capture : vers la droite", testCaptureRight);
    TestRunner::run("capture : vers la gauche", testCaptureLeft);
    TestRunner::run("capture : vers le haut", testCaptureUp);
    TestRunner::run("capture : diagonale ↖", testCaptureDiagonalUpLeft);
    TestRunner::run("capture : marche aussi pour Blanc", testCaptureForWhite);
    TestRunner::run("capture : deux paires d'un coup", testDoubleCapture);
    TestRunner::run("capture : les 8 directions d'un coup", testCaptureInAllEightDirections);
    TestRunner::run("capture : une pierre seule ne se capture pas", testSingleStoneIsNotCaptured);
    TestRunner::run("capture : trois pierres ne se capturent pas", testThreeStonesAreNotCaptured);
    TestRunner::run("capture : paire non fermée", testOpenPairIsNotCaptured);
    TestRunner::run("capture : paire fermée par l'adversaire", testPairClosedByOpponentIsNotCaptured);
    TestRunner::run("capture : sa propre pierre dans le motif", testOwnStoneInsideIsNotCaptured);
    TestRunner::run("capture : motif déjà présent ailleurs", testExistingPatternElsewhereIsIgnored);
    TestRunner::run("capture : pas d'auto-capture", testNoSelfCapture);
    TestRunner::run("capture : fermée contre le bord gauche", testCaptureClosedAgainstLeftEdge);
    TestRunner::run("capture : fermée contre le bord droit", testCaptureClosedAgainstRightEdge);
    TestRunner::run("capture : paire collée au bord droit", testPairAgainstRightEdgeIsNotCaptured);
    TestRunner::run("capture : paire collée au bord bas", testPairAgainstBottomEdgeIsNotCaptured);
    TestRunner::run("capture : paire collée au coin en diagonale", testPairAgainstCornerDiagonalIsNotCaptured);
    TestRunner::run("capture : dans le coin bas-droit", testCaptureInBottomRightCorner);
    TestRunner::run("capture : pierre dans chaque coin", testStoneInEachCornerDoesNotCrash);
    TestRunner::run("capture : balayage de tout le plateau", testEveryCellNextToOpponentsDoesNotCrash);
    TestRunner::run("capture : depuis une case vide", testCaptureFromEmptyCell);
    TestRunner::run("capture : exemple de l'annexe du sujet", testSubjectCaptureExample);

    TestRunner::run("double-trois : trois en diagonale (annexe)", testSubjectDiagonalFreeThree);
    TestRunner::run("double-trois : trois avec un trou (annexe)", testSubjectSplitFreeThree);
    TestRunner::run("double-trois : coup a interdit (annexe)", testSubjectDoubleThreeIsForbidden);
    TestRunner::run("double-trois : coup a légal avec une pierre en b (annexe)", testSubjectDoubleThreeBlockedInB);
    TestRunner::run("double-trois : un seul trois libre est autorisé", testSingleFreeThreeIsAllowed);
    TestRunner::run("double-trois : trois bloqué par l'adversaire", testThreeBlockedByOpponentIsNotFree);
    TestRunner::run("double-trois : marche aussi pour Blanc", testDoubleThreeForWhite);
    TestRunner::run("double-trois : autorisé s'il capture", testDoubleThreeByCaptureIsAllowed);
    TestRunner::run("double-trois : trois contre le bord haut", testThreeAgainstTopEdgeIsNotFree);
    TestRunner::run("double-trois : trois contre le bord gauche", testThreeAgainstLeftEdgeIsNotFree);
    TestRunner::run("double-trois : une case avant le coin", testDoubleThreeOneCellFromTheCorner);
    TestRunner::run("double-trois : collé au coin", testDoubleThreeAgainstTheCornerIsAllowed);

    TestRunner::run("réponse au 5 : rien à capturer", testFiveWithNothingToCaptureWins);
    TestRunner::run("réponse au 5 : une paire du 5 capturée", testFiveBrokenByCapture);
    TestRunner::run("réponse au 5 : la pierre testée est capturée", testCapturingTheCheckedStoneBreaksTheFive);
    TestRunner::run("réponse au 5 : un 6 qui perd un bout gagne encore", testSixLosingAnEndStoneStillWins);
    TestRunner::run("réponse au 5 : capture loin du 5", testCaptureAwayFromTheFiveDoesNotAnswer);
    TestRunner::run("réponse au 5 : capture loin du 5 à 8 captures", testCaptureAwayFromTheFiveWinsAtEightCaptures);
    TestRunner::run("réponse au 5 : 8 captures sans paire à prendre", testEightCapturesWithoutAPairToTakeDoesNotAnswer);
    TestRunner::run("réponse au 5 : 6 captures ne suffisent pas", testSixCapturesAreNotEnough);

    return TestRunner::finish();
}
