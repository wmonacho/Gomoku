#include "core/Board.hpp"

#include <random>

#include "support/TestRunner.hpp"

namespace {

    struct Snapshot {
        std::array<Cell, BOARD_CELLS> cells;
        int                           blackCaptureCount;
        int                           whiteCaptureCount;
        uint64_t                      hash;
    };

    Snapshot snapshotOf(const Board& board) {
        Snapshot snapshot;
        for (int y = 0; y < BOARD_SIZE; y++)
            for (int x = 0; x < BOARD_SIZE; x++)
                snapshot.cells[y * BOARD_SIZE + x] = board.at({x, y});
        snapshot.blackCaptureCount = board.captureCount(Cell::Black);
        snapshot.whiteCaptureCount = board.captureCount(Cell::White);
        snapshot.hash = board.hash();
        return snapshot;
    }

    bool sameState(const Snapshot& a, const Snapshot& b) {
        return a.cells == b.cells && a.blackCaptureCount == b.blackCaptureCount
            && a.whiteCaptureCount == b.whiteCaptureCount && a.hash == b.hash;
    }

    // Board n'applique pas les règles : on capture des voisines adverses au hasard pour exercer undo.
    // Le plafond de 10 reproduit une vraie partie, qui s'arrête là.
    void captureRandomNeighbors(Board& board, Pos placed, Cell color, std::mt19937& rng) {
        for (Pos direction : DIRECTIONS) {
            for (int sign : {1, -1}) {
                const Pos neighbor = step(placed, direction, sign);
                if (board.isInside(neighbor) && board.at(neighbor) == opponent(color)
                    && board.captureCount(color) < 10 && rng() % 3 == 0)
                    board.capture(neighbor);
            }
        }
    }

    // Joue jusqu'à moveCount coups aléatoires et renvoie le nombre de coups réellement posés.
    int playRandomMoves(Board& board, int moveCount, std::mt19937& rng) {
        int played = 0;
        Cell color = Cell::Black;
        for (int i = 0; i < moveCount; i++) {
            const Pos pos = {static_cast<int>(rng() % BOARD_SIZE), static_cast<int>(rng() % BOARD_SIZE)};
            if (board.at(pos) != Cell::Empty)
                continue;
            board.place(pos, color);
            captureRandomNeighbors(board, pos, color, rng);
            color = opponent(color);
            played++;
        }
        return played;
    }

    void testNewBoardIsEmpty() {
        const Board board;
        for (int y = 0; y < BOARD_SIZE; y++)
            for (int x = 0; x < BOARD_SIZE; x++)
                CHECK(board.at({x, y}) == Cell::Empty);
        CHECK(board.captureCount(Cell::Black) == 0);
        CHECK(board.captureCount(Cell::White) == 0);
        CHECK(board.hash() == board.computeHashFromScratch());
    }

    void testPlaceThenAt() {
        Board board;
        board.place({3, 4}, Cell::Black);
        board.place({18, 18}, Cell::White);
        CHECK(board.at({3, 4}) == Cell::Black);
        CHECK(board.at({18, 18}) == Cell::White);
        CHECK(board.at({4, 3}) == Cell::Empty);
        CHECK(board.at({0, 0}) == Cell::Empty);
    }

    void testIsInside() {
        const Board board;
        CHECK(board.isInside({0, 0}));
        CHECK(board.isInside({BOARD_SIZE - 1, BOARD_SIZE - 1}));
        CHECK(!board.isInside({-1, 0}));
        CHECK(!board.isInside({0, -1}));
        CHECK(!board.isInside({BOARD_SIZE, 0}));
        CHECK(!board.isInside({0, BOARD_SIZE}));
    }

    void testCaptureAndUndo() {
        Board board;
        board.place({1, 0}, Cell::Black);
        board.place({2, 0}, Cell::White);
        board.place({3, 0}, Cell::White);
        const Snapshot beforeCapture = snapshotOf(board);

        board.place({4, 0}, Cell::Black);
        board.capture({2, 0});
        board.capture({3, 0});
        CHECK(board.at({2, 0}) == Cell::Empty);
        CHECK(board.at({3, 0}) == Cell::Empty);
        CHECK(board.captureCount(Cell::Black) == 2);
        CHECK(board.captureCount(Cell::White) == 0);
        CHECK(board.hash() == board.computeHashFromScratch());

        board.undo();
        CHECK(sameState(snapshotOf(board), beforeCapture));
    }

    void testHashDependsOnSideToMove() {
        Board blackThenWhite;
        blackThenWhite.place({5, 5}, Cell::Black);
        blackThenWhite.place({6, 6}, Cell::White);

        Board sameStonesOneMoveMore;
        sameStonesOneMoveMore.place({5, 5}, Cell::Black);
        const uint64_t hashWithWhiteToMove = sameStonesOneMoveMore.hash();
        sameStonesOneMoveMore.place({6, 6}, Cell::White);

        CHECK(blackThenWhite.hash() == sameStonesOneMoveMore.hash());
        CHECK(hashWithWhiteToMove != blackThenWhite.hash());
        CHECK(Board().hash() != hashWithWhiteToMove);
    }

    void testHashDependsOnCaptureCount() {
        Board noCapture;
        noCapture.place({1, 0}, Cell::Black);

        // Mêmes pierres et même trait que noCapture, mais Noir a capturé 2 pierres.
        Board twoCaptures;
        twoCaptures.place({0, 0}, Cell::White);
        twoCaptures.place({2, 0}, Cell::White);
        twoCaptures.place({1, 0}, Cell::Black);
        twoCaptures.capture({0, 0});
        twoCaptures.capture({2, 0});

        CHECK(twoCaptures.captureCount(Cell::Black) == 2);
        CHECK(twoCaptures.at({0, 0}) == Cell::Empty && twoCaptures.at({2, 0}) == Cell::Empty);
        CHECK(twoCaptures.hash() != noCapture.hash());
    }

    // Critère du ticket : poser puis annuler N coups redonne exactement l'état et le hash de départ.
    void testPlaceThenUndoRestoresStartState() {
        std::mt19937 rng(42);
        Board board;
        playRandomMoves(board, 30, rng);

        for (int round = 0; round < 100; round++) {
            const Snapshot start = snapshotOf(board);
            const int played = playRandomMoves(board, 200, rng);
            CHECK(board.hash() == board.computeHashFromScratch());
            for (int i = 0; i < played; i++)
                board.undo();
            CHECK(sameState(snapshotOf(board), start));
        }
    }

    // Critère du ticket : aucune allocation dans place / capture / undo.
    void testNoAllocationInPlaceCaptureUndo() {
        std::mt19937 rng(7);
        Board board;

        const long allocationsBefore = TestRunner::allocationCount();
        for (int round = 0; round < 1000; round++) {
            const int played = playRandomMoves(board, 100, rng);
            for (int i = 0; i < played; i++)
                board.undo();
        }
        const long allocationsDuring = TestRunner::allocationCount() - allocationsBefore;

        CHECK(allocationsDuring == 0);
    }
}

int main() {
    TestRunner::printTitle("Tests de Board");

    TestRunner::run("plateau neuf vide", testNewBoardIsEmpty);
    TestRunner::run("place puis at", testPlaceThenAt);
    TestRunner::run("isInside sur les bords", testIsInside);
    TestRunner::run("capture puis undo", testCaptureAndUndo);
    TestRunner::run("le hash dépend du trait", testHashDependsOnSideToMove);
    TestRunner::run("le hash dépend des captures", testHashDependsOnCaptureCount);
    TestRunner::run("place/undo de N coups restaure l'état et le hash", testPlaceThenUndoRestoresStartState);
    TestRunner::run("aucune allocation dans place/capture/undo", testNoAllocationInPlaceCaptureUndo);

    return TestRunner::finish();
}
