#pragma once

// Passage entre un dessin ASCII et un Board, pour écrire des tests lisibles et afficher une position.
//   'X' = Noir, 'O' = Blanc, '.' ou '_' = vide.
//   La ligne i du dessin est y = i, le caractère j de la ligne est x = j.

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

#include "core/Board.hpp"

// Les cases au-delà du dessin restent vides. Chaque pierre passe par Board::place :
// l'historique et le trait du Board n'ont donc pas de sens après un boardFrom.
Board boardFrom(std::initializer_list<std::string_view> rows);

// Les 19 lignes du plateau, 19 caractères chacune, dans le même format que boardFrom.
std::vector<std::string> toRows(const Board& board);

// Plateau complet avec coordonnées et compteurs de captures, pour l'afficher quand un test échoue.
std::string toAscii(const Board& board);
void        printBoard(const Board& board);
