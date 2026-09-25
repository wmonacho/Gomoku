#pragma once

#include <optional>
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
    Board               board_;
    Cell                currentPlayer_;
    int                 blackCaptureCount_;
    int                 whiteCaptureCount_;
    std::optional<Cell> winner_;
};
