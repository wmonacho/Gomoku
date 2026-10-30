#include "core/Game.hpp"

#include "core/Rules.hpp"

Game::Game() {
    reset();
}

void Game::reset() {
    board_ = Board();
    currentPlayer_ = Cell::Black;
    winner_ = std::nullopt;

    pendingFive_ = std::nullopt;
    history_.clear();
}

MoveResult Game::play(Pos pos) {
    const std::string reason = rejectionReason(pos);
    if (!reason.empty()) {
        return {false, reason, {}};
    }

    history_.push_back({currentPlayer_, winner_, pendingFive_});

    board_.place(pos, currentPlayer_);
    const std::vector<Pos> captures = Rules::findCaptures(board_, pos, currentPlayer_);
    for (const Pos& capture : captures) {
        board_.capture(capture);
    }

    const bool opponentHadPendingFive = pendingFive_.has_value();
    pendingFive_ = std::nullopt;

    if (board_.captureCount(currentPlayer_) >= Rules::WINNING_CAPTURES) {
        winner_ = currentPlayer_;
    } else if (opponentHadPendingFive && Rules::hasFiveInARow(board_, opponent(currentPlayer_))) {
        winner_ = opponent(currentPlayer_);
    } else if (Rules::makesFiveInARow(board_, pos, currentPlayer_)) {
        if (!Rules::canAnswerFive(board_, pos, currentPlayer_))
            winner_ = currentPlayer_;
        else
            pendingFive_ = pos;
    }

    currentPlayer_ = opponent(currentPlayer_);
    return {true, "", captures};
}

void Game::undo() {
    board_.undo();
    const State& previous = history_.back();
    currentPlayer_ = previous.currentPlayer;
    winner_ = previous.winner;
    pendingFive_ = previous.pendingFive;
    history_.pop_back();
}

bool Game::isLegalMove(Pos pos) const {
    return rejectionReason(pos).empty();
}

std::string Game::rejectionReason(Pos pos) const {
    if (winner_) {
        return "The game is over";
    }
    if (!board_.isInside(pos)) {
        return "Position is outside the board";
    }
    if (board_.at(pos) != Cell::Empty) {
        return "Position is already occupied";
    }
    if (Rules::createsDoubleFreeThree(board_, pos, currentPlayer_)) {
        return "Double free-three is forbidden";
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
