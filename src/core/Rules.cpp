#include "core/Rules.hpp"

namespace Rules {

// TODO: définir ici les 4 directions (horizontale, verticale, 2 diagonales)
// dans un namespace anonyme, partagées par les trois fonctions.

std::vector<Pos> findCaptures(const Board& board, Pos pos, Cell color) {
    // TODO: dans les 8 sens depuis pos, chercher le motif color, adverse, adverse, color.
    // Seule une paire exacte est capturable : retourner les 2 pierres adverses de chaque motif trouvé.
    return {};
}

bool makesFiveInARow(const Board& board, Pos pos, Cell color) {
    // TODO: pour chaque direction, compter les pierres de color dans les deux sens + pos.
    // Vrai si le total atteint 5 ou plus.
    return false;
}

bool createsDoubleFreeThree(const Board& board, Pos pos, Cell color) {
    // TODO: compter les « trois libres » créés par pos (motifs _XXX_ et _XX_X_, dans chaque direction).
    // Vrai s'il y en a au moins 2. Exception du sujet : autorisé si le coup capture une paire.
    return false;
}

}  // namespace Rules
