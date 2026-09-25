<div align="center">

<img src="assets/banner.svg" alt="Goban 3D animé : les noirs alignent 5 pierres et gagnent" width="100%"/>

<a href="https://git.io/typing-svg"><img src="https://readme-typing-svg.demolab.com?font=Fira+Code&weight=600&size=22&pause=1000&color=F7B733&center=true&vCenter=true&width=600&lines=Yeah%2C+well%2C+your+brain+has+to+fry+sometime+%F0%9F%A7%A0%F0%9F%94%A5;19x19+Goban+%E2%80%A2+Captures+%E2%80%A2+No+double-threes;An+AI+that+thinks+in+under+0.5s+%E2%9A%A1" alt="Typing SVG" /></a>

<br/>

![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![SFML](https://img.shields.io/badge/SFML-graphics-8CC445?style=for-the-badge&logo=sfml&logoColor=white)
![ImGui](https://img.shields.io/badge/Dear-ImGui-1E1E1E?style=for-the-badge)
![42](https://img.shields.io/badge/42-School-000000?style=for-the-badge&logo=42&logoColor=white)

</div>

---

## 🎯 Le projet

**Gomoku** (五目並べ) est un jeu de stratégie traditionnellement joué sur un goban avec des pierres noires et blanches.
Le but de ce projet 42 : créer une **IA capable de battre des joueurs humains**, en moins d'une demi-seconde par coup.

<table>
<tr>
<td width="50%">

### ⚔️ Modes de jeu
- 🤖 **Joueur vs IA** : l'IA doit gagner, sans que tu la laisses faire
- 👥 **Joueur vs Joueur** : sur la même machine, avec suggestion de coups
- ⏱️ Chronomètre affichant le temps de réflexion de l'IA

</td>
<td width="50%">

### 🧠 Sous le capot
- 🌲 **Minimax** + élagage **Alpha-Bêta**
- 📊 Heuristique d'évaluation des alignements
- ⚡ Objectif : **< 0.5 s** par coup
- 🎨 Interface **SFML** + **Dear ImGui**

</td>
</tr>
</table>

---

## 📜 Les règles

| | Règle | Description |
|:-:|---|---|
| ⚫⚫⚫⚫⚫ | **Alignement** | Aligner **5 pierres ou plus** sur un goban **19×19** pour gagner |
| ✂️ | **Capture** | Encadrer une **paire** adverse pour la retirer. **10 pierres capturées = victoire** |
| 🛡️ | **Capture de fin de partie** | Un alignement de 5 ne gagne que si l'adversaire ne peut pas le casser en capturant une paire |
| 🚫 | **Pas de double-trois** | Interdit de jouer un coup créant deux *free-threes* simultanément |

<div align="center">

### ✂️ La capture en action

<img src="assets/capture.svg" alt="Capture animation" width="520"/>

</div>

---

## 🚀 Installation & lancement

```bash
git clone <repo-url> Gomoku && cd Gomoku
make        # compile le binaire
./gomoku    # lance le jeu 🎮
```

| Commande | Effet |
|---|---|
| `make` / `make all` | Compile le projet |
| `make clean` | Supprime les objets |
| `make fclean` | Supprime objets + binaire |
| `make re` | Recompile tout |

---

## 🗂️ Structure

```
Gomoku/
├── 📁 assets/          # Visuels du README
├── 📁 libs/            # SFML · Dear ImGui · ImGui-SFML
├── 📁 src/
│   ├── main.cpp
│   ├── 📁 ui/          # Menus & rendu SFML / ImGui
│   │   └── mainMenu
│   ├── 📁 app/         # Contrat entre l'UI et le moteur
│   │   ├── Dto.hpp     # Types échangés : Pos, Cell, MoveResult, AiMove, GameView
│   │   └── GameService # Façade : nouvelle partie, coups humain / IA, suggestion
│   └── 📁 core/        # Moteur de jeu, sans dépendance graphique
│       ├── Board       # Goban 19×19
│       ├── Rules       # Captures, alignement de 5, double-trois
│       ├── Game        # État de la partie : tour, captures, vainqueur, undo
│       └── 📁 ai/
│           ├── Search  # Minimax + Alpha-Bêta, limité en temps
│           └── Eval    # Heuristique d'évaluation
└── Makefile
```

L'UI ne parle qu'à `GameService` et ne manipule que les types de `Dto.hpp`. Le `core` peut ainsi évoluer sans toucher au rendu.

---

<div align="center">

<img src="https://capsule-render.vercel.app/api?type=waving&color=0:302b63,50:fc4a1a,100:f7b733&height=120&section=footer&text=Five%20in%20a%20row%20wins&fontSize=24&fontColor=ffffff&animation=twinkling" width="100%"/>

</div>
