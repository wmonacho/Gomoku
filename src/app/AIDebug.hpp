#pragma once

#include <string>

#include "app/Dto.hpp"

// Raisonnement de l'IA en texte lisible : coup choisi, ligne prévue, coups étudiés et leur score.
// Affiché dans le terminal quand le jeu est lancé avec GOMOKU_DEBUG=1, utilisable aussi par l'interface.
namespace AIDebug {

std::string describe(const MoveSuggestion& suggestion);

}  // namespace AIDebug
