#include "core/Rules.hpp"

#include <cassert>

namespace Rules {

std::vector<Pos> findCaptures(const Board& board, Pos pos, Cell color) {
    assert(board.isInside(pos) && "findCaptures() hors du plateau");
    assert(board.at(pos) == color && "findCaptures() s'appelle après avoir posé la pierre de color en pos");
    // TODO: dans les 8 sens depuis pos, chercher le motif color, adverse, adverse, color.
    // Seule une paire exacte est capturable : retourner les 2 pierres adverses de chaque motif trouvé.

    return {};
}

bool makesFiveInARow(const Board& board, Pos pos, Cell color) {
    assert(board.isInside(pos) && "makesFiveInARow() hors du plateau");
    assert(board.at(pos) == color && "makesFiveInARow() s'appelle après avoir posé la pierre de color en pos");
    // TODO: pour chaque direction, compter les pierres de color dans les deux sens + pos.
    // Vrai si le total atteint 5 ou plus.
    return false;
}

bool createsDoubleFreeThree(const Board& board, Pos pos, Cell color) {
    assert(board.isInside(pos) && "createsDoubleFreeThree() hors du plateau");
    assert(color != Cell::Empty && "createsDoubleFreeThree() demande la couleur du joueur");
    assert(board.at(pos) == Cell::Empty && "createsDoubleFreeThree() s'appelle avant de poser la pierre");
    // TODO: compter les « trois libres » créés par pos (motifs _XXX_ et _XX_X_, dans chaque direction).
    // Vrai s'il y en a au moins 2. Exception du sujet : autorisé si le coup capture une paire.
    return false;
}

}  // namespace Rules
