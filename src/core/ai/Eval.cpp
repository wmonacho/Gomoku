#include "core/ai/Eval.hpp"

namespace Eval {

int evaluate(const Game& game, Cell color) {
    // TODO:
    // 1. Victoire / défaite (winner) : retourner un très grand score positif / négatif.
    // 2. Pour chaque ligne du plateau, repérer les motifs (5, quatre ouvert, quatre fermé, trois ouvert...)
    //    et leur donner un poids.
    // 3. Ajouter les captures (proche de 10 = très dangereux) et les paires menacées.
    // 4. Score = total(color) - total(adversaire).
    return 0;
}

}  // namespace Eval
