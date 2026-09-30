#pragma once

// Outils communs à tous les fichiers tests/*Test.cpp : vérifications, affichage, compteur d'allocations.

#define CHECK(condition)                                                    \
    do {                                                                    \
        if (!(condition))                                                   \
            TestRunner::reportFailure(__FILE__, __LINE__, #condition);      \
    } while (0)

namespace TestRunner {

void printTitle(const char* title);
void run(const char* name, void (*test)());
// Affiche le bilan et renvoie le code de sortie du programme (0 si tout passe).
int  finish();

void reportFailure(const char* file, int line, const char* condition);
// Nombre d'appels à operator new depuis le lancement du programme.
long allocationCount();

}  // namespace TestRunner
