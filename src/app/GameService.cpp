#include "app/GameService.hpp"
#include "core/ai/SearchConfig.hpp"
#include <iostream>

#include <cassert>

namespace {

    void resetStats(PlayerInfo& player) {
        player.capturedStones = 0;
        player.lastMoveTime = 0.0f;
        player.lastMoveCoord = "-";
    }
}

GameService::GameService() : mode_(Mode::PvAI), difficulty_(SearchConfig::DEPTH) {}

void GameService::newGame(GameInfo& gameInfo) {
    assert(gameInfo.difficulty > 0 && "newGame() : difficulty doit être une profondeur positive");
    assert(gameInfo.p1Info.color != Cell::Empty && gameInfo.p2Info.color != Cell::Empty
           && "newGame() : chaque joueur doit avoir une couleur");
    assert(gameInfo.p1Info.color != gameInfo.p2Info.color && "newGame() : les deux joueurs ont la même couleur");

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.5);

    bool p1Starts = dist(gen);
    if (p1Starts)
        gameInfo.gameView.currentPlayer = &gameInfo.p1Info;
    else
        gameInfo.gameView.currentPlayer = &gameInfo.p2Info;

    mode_ = gameInfo.selectedMode;
    difficulty_ = gameInfo.difficulty;
    game_.reset(gameInfo.gameView.currentPlayer->color);
    search_.clear();

    resetStats(gameInfo.p1Info);
    resetStats(gameInfo.p2Info);

    const bool tipsEnable = gameInfo.gameView.tipsEnable;
    gameInfo.gameView.board.fill(Cell::Empty);

    gameInfo.gameView.tipsEnable = tipsEnable;
    gameInfo.gameView.gameStarted = true;
}

MoveResult GameService::playMove(Pos pos) {
    return game_.play(pos);
}

MoveSuggestion GameService::suggestMove() {
    if (game_.winner())
        return {NO_MOVE, 0.0, 0, 0, {}, StopReason::MaxDepth, false};
    return search_.findBestMove(game_, difficulty_, SearchConfig::TIME_LIMIT_MS);
}
// Est ce que c'est utiliser actuellement ? 
GameView GameService::currentView() const {
    GameView view;
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            view.board[y * BOARD_SIZE + x] = game_.board().at({x, y});

    view.winner = game_.winner();
    return view;
}

bool GameService::isLegalMove(Pos pos) const {
    return game_.isLegalMove(pos);
}
