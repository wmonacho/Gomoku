#include "app/GameService.hpp"

GameService::GameService() {
    // TODO: valeurs par défaut (ex. mode PvAI, humain Noir), puis newGame.
}

void GameService::newGame(Mode mode, Cell humanColor) {
    mode_ = mode;
    humanColor_ = humanColor;
    // TODO: game_.reset().
}

MoveResult GameService::playHumanMove(Pos pos) {
    // TODO: déléguer à game_.play(pos).
    return {};
}

AiMove GameService::playAiMove() {
    // TODO: search_.findBestMove(game_, limite), puis jouer le coup trouvé avec game_.play
    // et recopier les captures dans AiMove.
    return {};
}

Pos GameService::suggestMove() {
    // TODO: search_.findBestMove(game_, limite).pos, sans jouer le coup.
    return {};
}

GameView GameService::currentView() const {
    // TODO: remplir GameView depuis game_ (board case par case, currentPlayer, captureCount, winner).
    return {};
}

bool GameService::isLegalMove(Pos pos) const {
    return game_.isLegalMove(pos);
}
