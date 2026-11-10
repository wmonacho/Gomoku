#pragma once

#include <optional>
#include <vector>

#include "app/Dto.hpp"
#include "core/Board.hpp"

class Game {
public:
    Game();

    void                reset(Cell firstPlayer);
    MoveResult          play(Pos pos);
    // Pour l'IA : le coup vient de generateCandidates, déjà vérifié.
    void                playLegal(Pos pos);
    void                undo();
    bool                isLegalMove(Pos pos) const;

    const Board&        board() const;
    Cell                currentPlayer() const;
    int                 captureCount(Cell color) const;
    std::optional<Cell> winner() const;
    // Pourquoi la partie est finie (alignement, captures ou nulle) ; vide tant qu'elle continue.
    std::optional<WinReason> endReason() const;
    bool                isOver() const;
    std::optional<Pos>  pendingFive() const;

private:
    struct State {
        Cell                     currentPlayer;
        std::optional<Cell>      winner;
        std::optional<WinReason> endReason;
        std::optional<Pos>       pendingFive;
    };

    enum class MoveError { None, GameOver, Outside, Occupied, DoubleFreeThree };

    MoveError           checkMove(Pos pos) const;
    static const char*  describe(MoveError error);

    std::vector<Pos>    applyMove(Pos pos);
    bool                hasLegalMove() const;
    void                endGame(std::optional<Cell> winner, WinReason reason);

    Board               board_;
    Cell                currentPlayer_;
    std::optional<Cell> winner_;
    std::optional<WinReason> endReason_;

    std::optional<Pos>  pendingFive_;
    std::vector<State>  history_;
};
