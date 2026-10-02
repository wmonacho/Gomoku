#pragma once

#include <optional>
#include <string>
#include <vector>

#include "app/Dto.hpp"
#include "core/Board.hpp"

class Game {
public:
    Game();

    void                reset();
    MoveResult          play(Pos pos);
    void                undo();
    bool                isLegalMove(Pos pos) const;

    const Board&        board() const;
    Cell                currentPlayer() const;
    int                 captureCount(Cell color) const;
    std::optional<Cell> winner() const;

private:
    struct State {
        Cell                currentPlayer;
        std::optional<Cell> winner;
        std::optional<Pos>  pendingFive;
    };

    std::string         rejectionReason(Pos pos) const;

    Board               board_;
    Cell                currentPlayer_;
    std::optional<Cell> winner_;

    std::optional<Pos>  pendingFive_;
    std::vector<State>  history_;
};
