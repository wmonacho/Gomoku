#include "app/GameService.hpp"

#include <chrono>
#include <initializer_list>

#include "app/Dto.hpp"
#include "core/ai/SearchConfig.hpp"
#include "support/TestRunner.hpp"

namespace {

    void playAll(GameService& service, std::initializer_list<Pos> moves) {
        for (const Pos& move : moves)
            CHECK(service.playMove(move).isLegal);
    }

    // newGame tire le premier joueur au hasard : on renvoie sa couleur pour que les tests n'en dépendent pas.
    Cell startPvP(GameService& service, GameInfo& gameInfo) {
        gameInfo.selectedMode = Mode::PvP;
        service.newGame(gameInfo);
        return gameInfo.gameView.currentPlayer->color;
    }

    // Comme le menu : Noir (p1) humain et Blanc (p2) IA en PvAI, les deux IA en AIvAI.
    void startWithAI(GameService& service, GameInfo& gameInfo, Mode mode, Difficulty difficulty) {
        gameInfo.selectedMode = mode;
        gameInfo.difficulty = difficulty;
        gameInfo.p1Info.isAI = mode == Mode::AIvAI;
        gameInfo.p2Info.isAI = true;
        service.newGame(gameInfo);
    }

    bool isEmpty(const GameView& view) {
        for (Cell cell : view.board)
            if (cell != Cell::Empty)
                return false;
        return true;
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

    void testCurrentViewGivesTheWinReason() {
        GameService service;
        GameInfo gameInfo;
        startPvP(service, gameInfo);
        CHECK(!service.currentView().winReason);
        firstPlayerWinsByAlignment(service, gameInfo);
        CHECK(service.currentView().winReason == WinReason::Align);
    }

    void testNoAITurnBetweenHumans() {
        GameService service;
        GameInfo gameInfo;
        startPvP(service, gameInfo);
        CHECK(!service.isAITurn());
        CHECK(!service.playAITurn().result.isLegal);
    }

    // En PvAI, c'est à l'IA seulement quand le joueur courant est Blanc, quel que soit celui qui commence.
    void testAITurnFollowsTheCurrentPlayer() {
        GameService service;
        GameInfo gameInfo;
        startWithAI(service, gameInfo, Mode::PvAI, Difficulty::Easy);
        const bool aiStarts = gameInfo.gameView.currentPlayer->isAI;
        CHECK(service.isAITurn() == aiStarts);
        if (aiStarts)
            CHECK(service.playAITurn().result.isLegal);
        else
            playAll(service, {{9, 9}});
        CHECK(service.isAITurn() == !aiStarts);
    }

    void testPlayAITurnPlaysALegalMove() {
        GameService service;
        GameInfo gameInfo;
        startWithAI(service, gameInfo, Mode::AIvAI, Difficulty::Easy);
        const AIMove move = service.playAITurn();
        CHECK(move.result.isLegal);
        const Pos pos = move.suggestion.pos;
        CHECK(service.currentView().board[pos.y * BOARD_SIZE + pos.x] == gameInfo.gameView.currentPlayer->color);
        CHECK(!service.isAIThinking());
        CHECK(service.isAITurn());
    }

    // L'appelant continue de tourner pendant la recherche : chronomètre lisible, coups humains refusés.
    void testAITurnDoesNotBlock() {
        GameService service;
        GameInfo gameInfo;
        startWithAI(service, gameInfo, Mode::AIvAI, Difficulty::Easy);
        service.startAITurn();
        CHECK(service.isAIThinking());
        CHECK(!service.playMove({0, 0}).isLegal);
        CHECK(service.suggestMove().pos.x == NO_MOVE.x);

        std::optional<AIMove> move;
        const auto start = std::chrono::steady_clock::now();
        while (!move && std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
            CHECK(service.aiThinkingMs() >= 0.0);
            move = service.finishAITurn();
        }
        CHECK(move.has_value());
        CHECK(move && move->result.isLegal);
        CHECK(!service.isAIThinking());
        CHECK(service.aiThinkingMs() == 0.0);
        CHECK(!service.finishAITurn());
    }

    // Bonus : le niveau choisi fixe la profondeur de l'IA. Sans choix, c'est Difficile, le minimum du sujet.
    void testDifficultySetsTheDepth() {
        CHECK(GameInfo().difficulty == Difficulty::Hard);
        CHECK(SearchConfig::depthOf(Difficulty::Easy) == 3);
        CHECK(SearchConfig::depthOf(Difficulty::Medium) == 6);
        CHECK(SearchConfig::depthOf(Difficulty::Hard) == 10);

        for (Difficulty difficulty : {Difficulty::Easy, Difficulty::Medium}) {
            GameService service;
            GameInfo gameInfo;
            startWithAI(service, gameInfo, Mode::AIvAI, difficulty);
            playAll(service, {{9, 9}, {10, 10}, {9, 10}});
            CHECK(service.playAITurn().suggestion.depth == SearchConfig::depthOf(difficulty));
        }
    }

    // newGame arrête la recherche en cours au lieu d'attendre la fin de sa seconde de réflexion.
    void testNewGameStopsTheSearch() {
        GameService service;
        GameInfo gameInfo;
        startWithAI(service, gameInfo, Mode::AIvAI, Difficulty::Hard);
        playAll(service, {{9, 9}, {10, 10}, {9, 10}});
        service.startAITurn();

        const auto start = std::chrono::steady_clock::now();
        startWithAI(service, gameInfo, Mode::AIvAI, Difficulty::Hard);
        const auto elapsed = std::chrono::steady_clock::now() - start;
        CHECK(elapsed < std::chrono::milliseconds(300));
        CHECK(!service.isAIThinking());
        CHECK(isEmpty(service.currentView()));
    }
}

int main() {
    TestRunner::printTitle("GameService");

    TestRunner::run("suggestMove : partie en cours", testSuggestMoveDuringTheGame);
    TestRunner::run("suggestMove : partie terminée", testSuggestMoveAfterTheEndOfTheGame);
    TestRunner::run("NO_MOVE : refusé en cours de partie", testNoMoveIsRejectedDuringTheGame);
    TestRunner::run("currentView : raison de la victoire", testCurrentViewGivesTheWinReason);

    TestRunner::run("IA : jamais entre deux humains", testNoAITurnBetweenHumans);
    TestRunner::run("IA : son tour suit le joueur courant", testAITurnFollowsTheCurrentPlayer);
    TestRunner::run("IA : playAITurn joue un coup légal", testPlayAITurnPlaysALegalMove);
    TestRunner::run("IA : la recherche ne bloque pas l'appelant", testAITurnDoesNotBlock);
    TestRunner::run("IA : newGame arrête la recherche", testNewGameStopsTheSearch);
    TestRunner::run("IA : le niveau fixe la profondeur", testDifficultySetsTheDepth);

    return TestRunner::finish();
}
