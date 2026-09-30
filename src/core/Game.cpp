#include "core/Game.hpp"

#include "core/Rules.hpp"

Game::Game() {
    // TODO: appeler reset().
}

void Game::reset() {
    // TODO: plateau vide (board_ = Board()), Noir commence, pas de vainqueur.
}

MoveResult Game::play(Pos pos) {
    // TODO:
    // 1. Refuser si !isLegalMove(pos) (remplir rejectionReason).
    // 2. board_.place, puis board_.capture pour chaque pierre trouvée par Rules::findCaptures
    //    (Board enregistre tout pour undo et met à jour les compteurs).
    // 3. Détecter la victoire : 10 pierres capturées, ou 5 alignés non cassables par une capture adverse.
    // 4. Passer au joueur suivant.
    return {};
}

void Game::undo() {
    // TODO: board_.undo(), puis restaurer joueur courant et vainqueur.
}

bool Game::isLegalMove(Pos pos) const {
    // TODO: dans le plateau, case vide, partie pas finie, et pas de double-trois (Rules::createsDoubleFreeThree).
    return false;
}

const Board& Game::board() const {
    return board_;
}

Cell Game::currentPlayer() const {
    return currentPlayer_;
}

int Game::captureCount(Cell color) const {
    return board_.captureCount(color);
}

std::optional<Cell> Game::winner() const {
    return winner_;
}
