#pragma once

#include <vector>

#include "app/Dto.hpp"
#include "core/Board.hpp"

// Règles du sujet, sans état : chaque fonction lit le plateau et répond. C'est Game qui les applique.
namespace Rules {

constexpr int WINNING_CAPTURES = 10;

// Pierres adverses que color prend en jouant en pos (paires flanquées, 0, 2, 4... pierres).
std::vector<Pos> findCaptures(const Board& board, Pos pos, Cell color);
// Même test que findCaptures, sans construire la liste.
bool             canCapture(const Board& board, Pos pos, Cell color);
// La pierre de color en pos fait-elle partie d'un alignement de 5 ou plus ?
bool             makesFiveInARow(const Board& board, Pos pos, Cell color);
// Un alignement de 5 ou plus de color existe-t-il quelque part ?
bool             hasFiveInARow(const Board& board, Cell color);
// Fin de partie par capture : l'adversaire peut-il casser le 5 en prenant une paire, ou gagner par 10 captures ?
bool             canAnswerFive(const Board& board, Pos fivePos, Cell fiveColor);
// Jouer en pos crée-t-il deux trois libres (interdit), sauf si le coup capture ?
bool             createsDoubleFreeThree(const Board& board, Pos pos, Cell color);

}  // namespace Rules
