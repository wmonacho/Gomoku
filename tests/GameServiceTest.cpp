#include "app/GameService.hpp"

#include <initializer_list>

#include "app/Dto.hpp"
#include "support/TestRunner.hpp"

namespace {

    void playAll(GameService& service, std::initializer_list<Pos> moves) {
        for (const Pos& move : moves)
            CHECK(service.playMove(move).isLegal);
    }

    // newGame tire le premier joueur au hasard : on renvoie sa couleur pour que les tests n'en dépendent pas.
    Cell startPvP(GameService& service, GameInfo& gameInfo) {
        gameInfo.selectedMode = Mode::PvP;
        gameInfo.difficulty = 1;
        service.newGame(gameInfo);
        return gameInfo.gameView.currentPlayer->color;
    }

    // Le premier joueur aligne cinq pierres sur la ligne 0, l'autre joue loin sur la ligne 5.
    void firstPlayerWinsByAlignment(GameService& service, GameInfo& gameInfo) {
        const Cell firstPlayer = startPvP(service, gameInfo);
        playAll(service, {
            {0, 0}, {0, 5},
            {1, 0}, {2, 5},
            {2, 0}, {4, 5},
            {3, 0}, {6, 5},
            {4, 0},
        });
        CHECK(service.currentView().winner == firstPlayer);
    }

    void testSuggestMoveDuringTheGame() {
        GameService service;
        GameInfo gameInfo;
        startPvP(service, gameInfo);
        playAll(service, {{9, 9}});
        CHECK(service.isLegalMove(service.suggestMove().pos));
    }

    void testSuggestMoveAfterTheEndOfTheGame() {
        GameService service;
        GameInfo gameInfo;
        firstPlayerWinsByAlignment(service, gameInfo);
        const MoveSuggestion suggestion = service.suggestMove();
        CHECK(suggestion.pos.x == NO_MOVE.x && suggestion.pos.y == NO_MOVE.y);
        CHECK(!service.playMove(suggestion.pos).isLegal);
    }

    void testNoMoveIsRejectedDuringTheGame() {
        GameService service;
        GameInfo gameInfo;
        startPvP(service, gameInfo);
        CHECK(!service.playMove(NO_MOVE).isLegal);
    }
}

int main() {
    TestRunner::printTitle("GameService");

    TestRunner::run("suggestMove : partie en cours", testSuggestMoveDuringTheGame);
    TestRunner::run("suggestMove : partie terminée", testSuggestMoveAfterTheEndOfTheGame);
    TestRunner::run("NO_MOVE : refusé en cours de partie", testNoMoveIsRejectedDuringTheGame);

    return TestRunner::finish();
}
