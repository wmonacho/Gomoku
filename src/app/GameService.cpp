#include "app/GameService.hpp"

namespace {

    // Le sujet impose moins de 0,5 s en moyenne par coup.
    constexpr double SEARCH_TIME_LIMIT_MS = 500.0;
    // Le sujet impose au moins 10 niveaux de profondeur.
    constexpr int DEFAULT_DIFFICULTY = 10;

    void resetStats(PlayerInfo& player) {
        player.capturedStones = 0;
        player.lastMoveTime = 0.0f;
        player.lastMoveCoord = "-";
    }
}

GameService::GameService() : mode_(Mode::PvAI), difficulty_(DEFAULT_DIFFICULTY) {}

void GameService::newGame(Mode mode, int difficulty, GameInfo& gameInfo) {
    assert(difficulty > 0 && "newGame() : difficulty doit être une profondeur positive");
    assert(gameInfo.p1Info.color != Cell::Empty && gameInfo.p2Info.color != Cell::Empty
           && "newGame() : chaque joueur doit avoir une couleur");
    assert(gameInfo.p1Info.color != gameInfo.p2Info.color && "newGame() : les deux joueurs ont la même couleur");

    mode_ = mode;
    difficulty_ = difficulty;
    game_.reset();

    resetStats(gameInfo.p1Info);
    resetStats(gameInfo.p2Info);

    const bool tipsEnable = gameInfo.gameView.tipsEnable;
    gameInfo.gameView = currentView();
    gameInfo.gameView.gameStarted = true;
    gameInfo.gameView.tipsEnable = tipsEnable;
}

MoveResult GameService::playMove(Pos pos) {
    return game_.play(pos);
}

MoveSuggestion GameService::suggestMove() {
    return search_.findBestMove(game_, difficulty_, SEARCH_TIME_LIMIT_MS);
}
// Est ce que c'est utiliser actuellement ? 
GameView GameService::currentView() const {
    GameView view;
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            view.board[y * BOARD_SIZE + x] = game_.board().at({x, y});
    // view.currentPlayer = game_.currentPlayer();
    // view.currentPlayer = (game_.currentPlayer() == Cell::Black) ? &gameInfo.p1Info : &gameInfo.p2Info;
    // view.blackCaptureCount = game_.captureCount(Cell::Black); 
    // view.whiteCaptureCount = game_.captureCount(Cell::White);
    // view.winner = game_.winner();
    return view;
}

bool GameService::isLegalMove(Pos pos) const {
    return game_.isLegalMove(pos);
}
