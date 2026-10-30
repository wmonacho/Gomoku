#include "ui/BoardView.hpp"
#include <iostream>

BoardView::BoardView() {
    if (m_font.loadFromFile("assets/fonts/Monocraft.ttf")) {
        m_fontLoaded = true;
    } else {
        std::cerr << "Error: Failed to load font for board coordinates." << std::endl;
    }
}

void BoardView::handleInput(const sf::Event& event, const sf::RenderWindow& window, GameInfo& gameInfo) {
    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left || gameInfo.gameView.currentPlayer->isAI) {
        return;
    }

    // Ignore le clic si la souris survole une fenêtre ImGui (les panneaux latéraux)
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    ImVec2 windowSize(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y));
    ImVec2 centerPos(windowSize.x * 0.5f, windowSize.y * 0.44f);
    float boardSize = 640.0f;
    float margin = 35.0f;

    float startX = centerPos.x - (boardSize * 0.5f) + margin;
    float startY = centerPos.y - (boardSize * 0.5f) + margin;
    float step   = (boardSize - (margin * 2.0f)) / 18.0f;

    float mouseX = static_cast<float>(event.mouseButton.x);
    float mouseY = static_cast<float>(event.mouseButton.y);

    // Calcul de l'intersection la plus proche
    int gridX = std::round((mouseX - startX) / step);
    int gridY = std::round((mouseY - startY) / step);

    // Vérifie qu'on est bien sur la grille 19x19
    if (gridX < 0 || gridX >= 19 || gridY < 0 || gridY >= 19) {
        return;
    }

    // Vérifie que le clic est suffisamment proche du croisement (rayon de tolérance)
    float intersectionX = startX + gridX * step;
    float intersectionY = startY + gridY * step;
    float distSq = (mouseX - intersectionX) * (mouseX - intersectionX) +
                   (mouseY - intersectionY) * (mouseY - intersectionY);

    float clickRadius = step * 0.45f;
    if (distSq > clickRadius * clickRadius) {
        return;
    }

    int index = gridY * 19 + gridX;

    // Si la case est vide, on pose le pion
    if (gameInfo.gameView.board[index] == Cell::Empty) {
        gameInfo.gameView.board[index] = gameInfo.gameView.currentPlayer->color;
        // std::cout << "Index: " << index << std::endl;
        // Update last move
        std::string coordStr = std::to_string(gridX) + ":" + std::to_string(gridY);
        gameInfo.gameView.currentPlayer->lastMoveCoord = coordStr; 

        if (gameInfo.gameView.currentPlayer == &gameInfo.p1Info) {
            gameInfo.gameView.currentPlayer = &gameInfo.p2Info;
        } else {
            gameInfo.gameView.currentPlayer = &gameInfo.p1Info;
        }
    }
}

void BoardView::renderVictoryPopup(const PlayerInfo& winner, char reason) {
    std::cout << winner.name << " forfeited the match." << std::endl;
    // 1. Traduction du char en texte pour la raison
    std::string reasonStr;
    switch (reason) {
        case 'A':
        case '5':
            reasonStr = "align 5";
            break;
        case 'C':
        case '1':
            reasonStr = "10 captures";
            break;
        case 'F':
            reasonStr = "forfeit";
            break;
        case 'T':
            reasonStr = "timeout";
            break;
        default:
            reasonStr = std::string(1, reason);
            break;
    }

    // 2. Définition des textes à afficher
    std::string line1 = winner.name + " WIN";
    std::string line2 = "by " + reasonStr;

    // 3. Configuration de la fenêtre modale
    ImGui::OpenPopup("VictoryModal");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(280.0f, 150.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar 
                           | ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoCollapse;

    if (ImGui::BeginPopupModal("VictoryModal", nullptr, flags)) {
        ImGui::Spacing();

        // Ligne 1 : [PLAYER] WIN (centré)
        float w1 = ImGui::CalcTextSize(line1.c_str()).x;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - w1) * 0.5f);
        ImGui::Text("%s", line1.c_str());

        // Ligne 2 : by [REASON] (centré)
        float w2 = ImGui::CalcTextSize(line2.c_str()).x;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - w2) * 0.5f);
        ImGui::Text("%s", line2.c_str());

        ImGui::Spacing();
        ImGui::Spacing();

        // Ligne 3 : (menu) (rejouer)
        float btnWidth = 100.0f;
        float btnHeight = 32.0f;
        float spacing = 15.0f;
        float totalWidth = (btnWidth * 2.0f) + spacing;

        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - totalWidth) * 0.5f);

        // Bouton (menu)
        if (ImGui::Button("menu", ImVec2(btnWidth, btnHeight))) {
            // Réinitialise l'état pour retourner au menu
            // m_gameInfo->gameView.board.fill(Cell::Empty);
            // m_gameInfo->gameView.winner.reset();
            // m_gameInfo->gameView.gameStarted = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0f, spacing);

        // Bouton (rejouer)
        // if (ImGui::Button("rejouer", ImVec2(btnWidth, btnHeight))) {
        //     // Relance directement une nouvelle partie
        //     m_gameInfo->gameView.board.fill(Cell::Empty);
        //     m_gameInfo->p1Info.capturedStones = 0;
        //     m_gameInfo->p2Info.capturedStones = 0;
        //     m_gameInfo->p1Info.lastMoveCoord = "-";
        //     m_gameInfo->p2Info.lastMoveCoord = "-";
        //     m_gameInfo->gameView.winner.reset();
        //     m_gameInfo->gameView.currentPlayer = &m_gameInfo->p1Info;
        //     ImGui::CloseCurrentPopup();
        // }

        ImGui::EndPopup();
    }
}

void BoardView::renderPlayerPanel(const std::string& panelId, const PlayerInfo& player, 
                                 bool isLeft, const ImVec2& windowSize, bool tipsEnable) {
    float panelWidth = 280.0f;
    float panelHeight = windowSize.y - 40.0f;
    float posX = isLeft ? 20.0f : (windowSize.x - panelWidth - 20.0f);
    float posY = 20.0f;

    ImGui::SetNextWindowPos(ImVec2(posX, posY));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin(player.name.c_str(), nullptr, flags);

    // Titre / Joueur
    ImGui::Separator();
    ImGui::Spacing();

    // Statistiques du joueur
    ImGui::Text("Type: %s", player.isAI ? "AI Engine" : "Human");
    ImGui::Spacing();

    ImGui::Text("Captured stones: %d / 10", player.capturedStones);
    ImGui::Text("Last move: %s", player.lastMoveCoord.c_str());

    if (player.isAI) {
        ImGui::Text("AI execution time: %.3f s", player.lastMoveTime);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    //TODO Remplit le best move
    if (tipsEnable & !player.isAI) {
        ImGui::Spacing();
        ImGui::Text("Next best move : ");
        ImGui::Spacing();
        ImGui::Separator();
    }
    // Bouton de forfait
    if (!player.isAI) {
        if (ImGui::Button("Forfeit", ImVec2(-1.0f, 35.0f))) {
            // std::cout << player.name << " forfeited the match." << std::endl;
            renderVictoryPopup(player, 'F');
        }
    }

    ImGui::End();
}

void BoardView::renderBoard(sf::RenderWindow& window, const ImVec2& centerPos, float boardSize, const GameView& view) {
    // 1. Fond en bois du plateau
    sf::RectangleShape goban(sf::Vector2f(boardSize, boardSize));
    goban.setOrigin(boardSize * 0.5f, boardSize * 0.5f);
    goban.setPosition(centerPos.x, centerPos.y);
    goban.setFillColor(sf::Color(220, 179, 92));
    goban.setOutlineThickness(3.0f);
    goban.setOutlineColor(sf::Color(40, 25, 10));
    window.draw(goban);

    // 2. Dimensions et marges
    float margin = 35.0f;
    float startX = centerPos.x - (boardSize * 0.5f) + margin;
    float startY = centerPos.y - (boardSize * 0.5f) + margin;
    float step   = (boardSize - (margin * 2.0f)) / 18.0f;

    // 3. Tracé des lignes de la grille
    for (int i = 0; i < 19; ++i) {
        float currentX = startX + i * step;
        float currentY = startY + i * step;

        sf::Vertex hLine[] = {
            sf::Vertex(sf::Vector2f(startX, currentY), sf::Color::Black),
            sf::Vertex(sf::Vector2f(startX + 18.0f * step, currentY), sf::Color::Black)
        };
        window.draw(hLine, 2, sf::Lines);

        sf::Vertex vLine[] = {
            sf::Vertex(sf::Vector2f(currentX, startY), sf::Color::Black),
            sf::Vertex(sf::Vector2f(currentX, startY + 18.0f * step), sf::Color::Black)
        };
        window.draw(vLine, 2, sf::Lines);
    }

    // 4. Police persistante et statique
    static sf::Font font;
    static bool fontReady = false;
    static bool fontAttempted = false;

    if (!fontAttempted) {
        fontAttempted = true;
        fontReady = font.loadFromFile("assets/fonts/Monocraft.ttf");
        if (!fontReady) {
            std::cerr << "Error: Failed to load font 'assets/fonts/Monocraft.ttf' for BoardView" << std::endl;
        }
    }

    if (!fontReady) {
        return;
    }

    // 5. Affichage des chiffres 0 à 18
    sf::Text coordText;
    coordText.setFont(font);
    coordText.setCharacterSize(12);
    coordText.setFillColor(sf::Color(50, 30, 15));

    for (int i = 0; i < 19; ++i) {
        float currentX = startX + i * step;
        float currentY = startY + i * step;

        coordText.setString(std::to_string(i));

        sf::FloatRect bounds = coordText.getLocalBounds();
        coordText.setOrigin(bounds.left + bounds.width * 0.5f,
                            bounds.top + bounds.height * 0.5f);

        // Axe horizontal (Haut et Bas)
        coordText.setPosition(currentX, startY - 18.0f);
        window.draw(coordText);

        coordText.setPosition(currentX, startY + 18.0f * step + 18.0f);
        window.draw(coordText);

        // Axe vertical (Gauche et Droite)
        coordText.setPosition(startX - 18.0f, currentY);
        window.draw(coordText);

        coordText.setPosition(startX + 18.0f * step + 18.0f, currentY);
        window.draw(coordText);
    }

    // Previsuatlisation des pierres
    // test v2
    static sf::Texture blackStoneTex;
    static sf::Texture whiteStoneTex;
    static bool texturesLoaded = false;

    if (!texturesLoaded) {
        const unsigned int size = 16;
        const float radius = 7.0f;
        const float center = 7.5f;

        sf::Image blackImg;
        sf::Image whiteImg;
        blackImg.create(size, size, sf::Color::Transparent);
        whiteImg.create(size, size, sf::Color::Transparent);

        for (unsigned int y = 0; y < size; ++y) {
            for (unsigned int x = 0; x < size; ++x) {
                float dx = static_cast<float>(x) - center;
                float dy = static_cast<float>(y) - center;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist <= radius) {
                    if (dist > radius - 1.2f) {
                        blackImg.setPixel(x, y, sf::Color(10, 10, 10));
                        whiteImg.setPixel(x, y, sf::Color(140, 140, 135));
                    } else if (x >= 4 && x <= 6 && y >= 4 && y <= 6) {
                        blackImg.setPixel(x, y, sf::Color(90, 90, 95));
                        whiteImg.setPixel(x, y, sf::Color(255, 255, 255));
                    } else {
                        blackImg.setPixel(x, y, sf::Color(35, 35, 38));
                        whiteImg.setPixel(x, y, sf::Color(220, 220, 215));
                    }
                }
            }
        }

        blackStoneTex.loadFromImage(blackImg);
        whiteStoneTex.loadFromImage(whiteImg);
        blackStoneTex.setSmooth(false);
        whiteStoneTex.setSmooth(false);

        texturesLoaded = true;
    }

    // 7. Affichage des pions pixelisés
    sf::Sprite stoneSprite;
    stoneSprite.setOrigin(8.0f, 8.0f);

    float targetDiameter = step * 0.85f;
    float scale = targetDiameter / 16.0f;
    stoneSprite.setScale(scale, scale);

    for (int y = 0; y < 19; ++y) {
        for (int x = 0; x < 19; ++x) {
            int index = y * 19 + x;
            Cell cell = view.board[index];

            if (cell == Cell::Empty) {
                continue;
            }

            if (cell == Cell::Black) {
                stoneSprite.setTexture(blackStoneTex);
            } else if (cell == Cell::White) {
                stoneSprite.setTexture(whiteStoneTex);
            }

            stoneSprite.setPosition(startX + x * step, startY + y * step);
            window.draw(stoneSprite);
        }
    }
    // fin test v2
    // 8. Prévisualisation sous le curseur (Ghost Stone)
    if (!ImGui::GetIO().WantCaptureMouse && view.currentPlayer != nullptr && !view.currentPlayer->isAI) {
        sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
        float mouseX = static_cast<float>(mousePixel.x);
        float mouseY = static_cast<float>(mousePixel.y);

        int hoverX = std::round((mouseX - startX) / step);
        int hoverY = std::round((mouseY - startY) / step);

        // Vérification des limites de la grille 19x19
        if (hoverX >= 0 && hoverX < 19 && hoverY >= 0 && hoverY < 19) {
            float targetX = startX + hoverX * step;
            float targetY = startY + hoverY * step;

            float distSq = (mouseX - targetX) * (mouseX - targetX) +
                           (mouseY - targetY) * (mouseY - targetY);
            float snapRadius = step * 0.45f;

            int hoverIndex = hoverY * 19 + hoverX;

            // Si la souris est proche du croisement et que la case est vide
            if (distSq <= snapRadius * snapRadius && view.board[hoverIndex] == Cell::Empty) {
                sf::Sprite ghostSprite;
                ghostSprite.setOrigin(8.0f, 8.0f);
                ghostSprite.setScale(scale, scale);

                if (view.currentPlayer->color == Cell::Black) {
                    ghostSprite.setTexture(blackStoneTex);
                } else {
                    ghostSprite.setTexture(whiteStoneTex);
                }

                // Opacité à ~50% pour marquer la prévisualisation
                ghostSprite.setColor(sf::Color(255, 255, 255, 130));
                ghostSprite.setPosition(targetX, targetY);

                window.draw(ghostSprite);
            }
        }
    }
}

void BoardView::renderBottomControls(const ImVec2& windowSize, GameView& view) {
    float btnWidth = 200.0f;
    float btnHeight = 35.0f;

    ImGui::SetNextWindowPos(ImVec2((windowSize.x - btnWidth) * 0.5f, windowSize.y - 65.0f));
    ImGui::SetNextWindowSize(ImVec2(btnWidth, btnHeight + 10.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar 
                           | ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoBackground;
 
    ImGui::Begin("BottomControls", nullptr, flags);
    if (ImGui::Button("Return to Menu", ImVec2(btnWidth, btnHeight))) {
        view.gameStarted = false;
    }
    ImGui::End();
}

void BoardView::render(sf::RenderWindow& window, GameInfo& gameInfo) {
    ImVec2 windowSize(static_cast<float>(window.getSize().x), 
                      static_cast<float>(window.getSize().y));

    // 1. Dessiner le Goban au centre (taille carrée ~650x650)
    renderBoard(window, ImVec2(windowSize.x * 0.5f, windowSize.y * 0.44f), 640.0f, gameInfo.gameView);

    // 2. Panneaux latéraux ImGui
    renderPlayerPanel("BlackPlayerPanel", gameInfo.p1Info, true, windowSize, gameInfo.gameView.tipsEnable);
    renderPlayerPanel("WhitePlayerPanel", gameInfo.p2Info, false, windowSize, gameInfo.gameView.tipsEnable);

    // 3. Bouton retour au menu en bas
    renderBottomControls(windowSize, gameInfo.gameView);
}