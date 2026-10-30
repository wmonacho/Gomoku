#include "app/GameService.hpp"

namespace {

    // Le sujet impose moins de 0,5 s en moyenne par coup.
    constexpr double SEARCH_TIME_LIMIT_MS = 500.0;
}

GameService::GameService() {
    newGame(Mode::PvAI, Cell::Black);
}

void GameService::newGame(Mode mode, Cell humanColor) {
    assert(humanColor != Cell::Empty && "newGame() : humanColor doit être Black ou White");
    mode_ = mode;
    humanColor_ = humanColor;
    game_.reset();
}

MoveResult GameService::playMove(Pos pos) {
    return game_.play(pos);
}

MoveSuggestion GameService::suggestMove() {
    return search_.findBestMove(game_, SEARCH_TIME_LIMIT_MS);
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
