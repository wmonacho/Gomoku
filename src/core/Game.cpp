#include "core/Game.hpp"

#include "core/Rules.hpp"

#include <cassert>

// État valide par défaut : la vraie couleur de départ arrive par reset().
Game::Game() {
    reset(Cell::Black);
}

void Game::reset(Cell firstPlayer) {
    assert(firstPlayer != Cell::Empty && "reset() : firstPlayer doit être Black ou White");
    board_ = Board();
    currentPlayer_ = firstPlayer;
    winner_ = std::nullopt;
    endReason_ = std::nullopt;

    pendingFive_ = std::nullopt;
    history_.clear();
}

MoveResult Game::play(Pos pos) {
    const MoveError error = checkMove(pos);
    if (error != MoveError::None) {
        return {false, describe(error), {}};
    }
    const std::vector<Pos> captures = applyMove(pos);
    // Seulement ici, pas dans playLegal : chercher un coup légal parcourt le plateau, trop cher pour la recherche.
    if (!endReason_ && !hasLegalMove())
        endGame(std::nullopt, WinReason::Draw);
    return {true, "", captures};
}

void Game::playLegal(Pos pos) {
    assert(isLegalMove(pos) && "playLegal() attend un coup déjà vérifié");
    applyMove(pos);
}

std::vector<Pos> Game::applyMove(Pos pos) {
    history_.push_back({currentPlayer_, winner_, endReason_, pendingFive_});

    board_.place(pos, currentPlayer_);
    const std::vector<Pos> captures = Rules::findCaptures(board_, pos, currentPlayer_);
    for (const Pos& capture : captures) {
        board_.capture(capture);
    }

    const bool opponentHadPendingFive = pendingFive_.has_value();
    pendingFive_ = std::nullopt;

    if (board_.captureCount(currentPlayer_) >= Rules::WINNING_CAPTURES) {
        endGame(currentPlayer_, WinReason::Captures);
    } else if (opponentHadPendingFive && Rules::hasFiveInARow(board_, opponent(currentPlayer_))) {
        endGame(opponent(currentPlayer_), WinReason::Align);
    } else if (Rules::makesFiveInARow(board_, pos, currentPlayer_)) {
        if (!Rules::canAnswerFive(board_, pos, currentPlayer_))
            endGame(currentPlayer_, WinReason::Align);
        else
            pendingFive_ = pos;
    }

    currentPlayer_ = opponent(currentPlayer_);
    return captures;
}

void Game::undo() {
    board_.undo();
    const State& previous = history_.back();
    currentPlayer_ = previous.currentPlayer;
    winner_ = previous.winner;
    endReason_ = previous.endReason;
    pendingFive_ = previous.pendingFive;
    history_.pop_back();
}

void Game::endGame(std::optional<Cell> winner, WinReason reason) {
    winner_ = winner;
    endReason_ = reason;
}

bool Game::hasLegalMove() const {
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            if (isLegalMove({x, y}))
                return true;
    return false;
}

bool Game::isLegalMove(Pos pos) const {
    return checkMove(pos) == MoveError::None;
}

Game::MoveError Game::checkMove(Pos pos) const {
    if (endReason_) {
        return MoveError::GameOver;
    }
    if (!board_.isInside(pos)) {
        return MoveError::Outside;
    }
    if (board_.at(pos) != Cell::Empty) {
        return MoveError::Occupied;
    }
    if (Rules::createsDoubleFreeThree(board_, pos, currentPlayer_)) {
        return MoveError::DoubleFreeThree;
    }
    return MoveError::None;
}

const char* Game::describe(MoveError error) {
    switch (error) {
        case MoveError::None:            return "";
        case MoveError::GameOver:        return "The game is over";
        case MoveError::Outside:         return "Position is outside the board";
        case MoveError::Occupied:        return "Position is already occupied";
        case MoveError::DoubleFreeThree: return "Double free-three is forbidden";
    }
    return "";
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

std::optional<WinReason> Game::endReason() const {
    return endReason_;
}

bool Game::isOver() const {
    return endReason_.has_value();
}

std::optional<Pos> Game::pendingFive() const {
    return pendingFive_;
}
