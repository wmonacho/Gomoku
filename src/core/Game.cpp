#include "core/Game.hpp"

#include "core/Rules.hpp"

Game::Game() {
    // TODO: appeler reset().
}

void Game::reset() {
    // TODO: plateau vide, Noir commence, compteurs à 0, pas de vainqueur, historique vidé.
}

MoveResult Game::play(Pos pos) {
    // TODO:
    // 1. Refuser si !isLegalMove(pos) (remplir rejectionReason).
    // 2. Poser la pierre, appliquer Rules::findCaptures, retirer les pierres, mettre à jour le compteur.
    // 3. Enregistrer le coup + captures dans l'historique (pour undo).
    // 4. Détecter la victoire : 10 pierres capturées, ou 5 alignés non cassables par une capture adverse.
    // 5. Passer au joueur suivant.
    return {};
}

void Game::undo() {
    // TODO: dépiler le dernier coup de l'historique, retirer la pierre, reposer les pierres capturées,
    // restaurer compteur, joueur courant et vainqueur.
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
    return color == Cell::Black ? blackCaptureCount_ : whiteCaptureCount_;
}

std::optional<Cell> Game::winner() const {
    return winner_;
}
