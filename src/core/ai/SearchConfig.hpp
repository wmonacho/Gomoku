#pragma once

// Réglages partagés par le jeu (GameService) et le bench : une seule valeur, pas de copie à oublier.
namespace SearchConfig {

    // Le sujet impose moins de 0,5 s en *moyenne* par coup, pas au maximum. La plupart des coups finissent la
    // profondeur 10 bien avant : seuls les coups difficiles vont jusqu'à la limite. Mesuré avec make bench :
    // 1000 ms donne ~210 ms de moyenne (~300 ms sur les 10 premiers coups) et un seul coup sur ~220 à profondeur 9.
    inline constexpr double TIME_LIMIT_MS = 1000.0;
    // Le sujet impose au moins 10 niveaux de profondeur.
    inline constexpr int    DEPTH = 10;

}
