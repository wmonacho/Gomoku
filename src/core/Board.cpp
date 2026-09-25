#include "core/Board.hpp"

Board::Board() {
    // TODO: initialiser toutes les intersections à Cell::Empty.
}

Cell Board::at(Pos pos) const {
    // TODO: lire la case (index = y * BOARD_SIZE + x si tableau).
    return Cell::Empty;
}

void Board::place(Pos pos, Cell color) {
    // TODO: écrire color dans la case. Pas de vérification de règles ici, c'est le rôle de Rules/Game.
}

void Board::remove(Pos pos) {
    // TODO: remettre la case à Cell::Empty (utilisé par les captures et undo).
}

bool Board::isInside(Pos pos) const {
    // TODO: 0 <= x < BOARD_SIZE et 0 <= y < BOARD_SIZE.
    return false;
}
