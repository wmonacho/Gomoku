#include "ui/BoardView.hpp"
#include <iostream>

BoardView::BoardView() {
    if (m_font.loadFromFile("assets/fonts/Monocraft.ttf")) {
        m_fontLoaded = true;
    } else {
        std::cerr << "Error: Failed to load font for board coordinates." << std::endl;
    }
}

void BoardView::triggerCaptureAnimations(const std::vector<Pos>& captures, Cell capturedColor) {
    for (const Pos& p : captures) {
        m_captureAnims.push_back({p, capturedColor, 0.0f, 0.30f});
    }
}

void BoardView::checkMove(MoveResult result, Pos targetPos, GameInfo& gameInfo, GameService& gameService) {
    if (result.isLegal) {
        if (!result.captured.empty()) {
            Cell capturedColor = (gameInfo.gameView.currentPlayer->color == Cell::Black) 
                                ? Cell::White 
                                : Cell::Black;

            triggerCaptureAnimations(result.captured, capturedColor);
        }
        std::string coordStr = std::to_string(targetPos.x) + ":" + std::to_string(targetPos.y);
        gameInfo.gameView.currentPlayer->lastMoveCoord = coordStr;

        if (!result.captured.empty()) {
            gameInfo.gameView.currentPlayer->capturedStones += static_cast<int>(result.captured.size());
        }

        // Synchronise le plateau complet (prend en compte le coup et les pions capturés retirés)
        GameView updatedView = gameService.currentView();
        gameInfo.gameView.board = updatedView.board;
        gameInfo.gameView.winner = updatedView.winner;

        if (gameInfo.gameView.winner.has_value()) {
            m_winnerInfo = (*gameInfo.gameView.winner == Cell::Black) ? &gameInfo.p1Info : & gameInfo.p2Info;
            if (gameInfo.gameView.currentPlayer->capturedStones >= 10)
                m_victoryReason = 'C';
            else if (gameInfo.gameView.currentPlayer->isAI && gameInfo.gameView.currentPlayer->lastMoveTime > 500)
                m_victoryReason = 'T';
            else
                m_victoryReason = 'A';
            m_showVictoryPopup = true;
        }
        if (gameInfo.gameView.currentPlayer == &gameInfo.p1Info) {
            gameInfo.gameView.currentPlayer = &gameInfo.p2Info;
        } else {
            gameInfo.gameView.currentPlayer = &gameInfo.p1Info;
        }
    } else {
        std::cout << "Coup illegal (" << targetPos.x << ", " << targetPos.y << ") : " 
                  << result.rejectionReason << std::endl;
    }
}

void BoardView::handleInput(const sf::Event& event, const sf::RenderWindow& window, 
                         GameInfo& gameInfo, GameService& gameService) {
    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left) {
        return;
    } else if (gameInfo.gameView.currentPlayer == nullptr || gameInfo.gameView.currentPlayer->isAI) {
        return;
    } else if (ImGui::GetIO().WantCaptureMouse) { // Ignore le clic si la souris survole une fenêtre ImGui
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

    int gridX = std::round((mouseX - startX) / step);
    int gridY = std::round((mouseY - startY) / step);

    if (gridX < 0 || gridX >= 19 || gridY < 0 || gridY >= 19) {
        return;
    }

    float intersectionX = startX + gridX * step;
    float intersectionY = startY + gridY * step;
    float distSq = (mouseX - intersectionX) * (mouseX - intersectionX) +
                   (mouseY - intersectionY) * (mouseY - intersectionY);

    float clickRadius = step * 0.45f;
    if (distSq > clickRadius * clickRadius) {
        return;
    }

    Pos targetPos{gridX, gridY};
    MoveResult result = gameService.playMove(targetPos);

    BoardView::checkMove(result, targetPos, gameInfo, gameService);
}

void BoardView::infoPopup(const PlayerInfo& winner, std::string reasonStr, GameInfo& gameInfo, GameService& gameService) {
    std::string line1 = winner.name + " WIN";
    std::string line2 = "by " + reasonStr;
    
    ImGui::Spacing();

    float w1 = ImGui::CalcTextSize(line1.c_str()).x;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - w1) * 0.5f);
    ImGui::Text("%s", line1.c_str());

    float w2 = ImGui::CalcTextSize(line2.c_str()).x;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - w2) * 0.5f);
    ImGui::Text("%s", line2.c_str());

    ImGui::Spacing();
    ImGui::Spacing();

    float btnWidth = 100.0f;
    float btnHeight = 32.0f;
    float spacing = 15.0f;
    float totalWidth = (btnWidth * 2.0f) + spacing;

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - totalWidth) * 0.5f);

    if (ImGui::Button("menu", ImVec2(btnWidth, btnHeight))) {
        gameInfo.gameView.gameStarted = false;
        m_winnerInfo = nullptr;
        m_showVictoryPopup = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine(0.0f, spacing);

    if (ImGui::Button("Play Again", ImVec2(btnWidth, btnHeight))) {
        m_winnerInfo = nullptr;
        gameService.newGame(gameInfo);
        m_showVictoryPopup = false; 
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void BoardView::renderVictoryPopup(const PlayerInfo& winner, char reason, GameInfo& gameInfo, GameService& gameService) {
    if (m_showVictoryPopup) {
        ImGui::OpenPopup("VictoryModal");
    }
    std::string reasonStr;
    switch (reason) {
        case 'A': reasonStr = "align 5"; break;
        case 'C': reasonStr = "10 captures"; break;
        case 'F': reasonStr = "forfeit"; break;
        case 'T': reasonStr = "timeout"; break;
        case 'O': reasonStr = "winning"; break;
        default:  reasonStr = std::string(1, reason); break;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(280.0f, 150.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar 
                           | ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoCollapse;

    if (ImGui::BeginPopupModal("VictoryModal", nullptr, flags))
        BoardView::infoPopup(winner, reasonStr, gameInfo, gameService);
}

void BoardView::renderPlayerPanel(const std::string& panelId, const PlayerInfo& player, 
                                 bool isLeft, const ImVec2& windowSize, bool tipsEnable, GameInfo& gameInfo) {
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

    if (tipsEnable & !player.isAI) {
        ImGui::Spacing();
        ImGui::Text("Next best move : ");
        ImGui::Spacing();
        ImGui::Separator();
    }

    if (!player.isAI) {
        if (ImGui::Button("Forfeit", ImVec2(-1.0f, 35.0f))) {
            if (player.color == gameInfo.p1Info.color)
                m_winnerInfo = &gameInfo.p2Info;
            else
                m_winnerInfo = &gameInfo.p1Info;
            m_victoryReason = 'F';
            m_showVictoryPopup = true;
        }
    }

    ImGui::End();
}

void BoardView::renderNumber(sf::RenderWindow& window, float startX, float startY, float step) {
    // Police persistante et statique
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

    // Affichage des chiffres 0 à 18
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
}

void BoardView::loadTextures(sf::Texture& blackStoneTex, sf::Texture& whiteStoneTex) {
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
}

void BoardView::captureStones(sf::RenderWindow& window, float startX, float startY, float step) {
    float dt = 1.0f / 60.0f; // Ajuste ou passe le deltaTime de ta frame si disponible

    for (auto it = m_captureAnims.begin(); it != m_captureAnims.end(); ) {
        it->elapsed += dt;

        // Fin de l'animation pour cette pierre
        if (it->elapsed >= it->duration) {
            it = m_captureAnims.erase(it);
            continue;
        }

        // Progression de 0.0f à 1.0f
        float t = it->elapsed / it->duration;
        float scale = 1.0f - t;
        sf::Uint8 alpha = static_cast<sf::Uint8>(255.0f * (1.0f - t));

        // Position de l'intersection correspondante sur le plateau
        float posX = startX + it->pos.x * step;
        float posY = startY + it->pos.y * step;

        // Dessin de la pierre animée (rétrécissement et disparition progressive)
        float radius = (step * 0.45f) * scale;
        sf::CircleShape stone(radius);
        stone.setOrigin(radius, radius);
        stone.setPosition(posX, posY);

        if (it->color == Cell::Black) {
            stone.setFillColor(sf::Color(20, 20, 20, alpha));
        } else {
            stone.setFillColor(sf::Color(240, 240, 240, alpha));
        }

        window.draw(stone);
        ++it;
    }
}

void BoardView::previewStone(sf::RenderWindow& window, float startX, float startY, float step, sf::Texture blackStoneTex, sf::Texture whiteStoneTex, const GameView& view) {
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
    BoardView::captureStones(window, startX, startY, step);
    BoardView::renderNumber(window, startX, startY, step);

    static sf::Texture blackStoneTex;
    static sf::Texture whiteStoneTex;
    static bool texturesLoaded = false;

    if (!texturesLoaded) {
        BoardView::loadTextures(blackStoneTex, whiteStoneTex);
        texturesLoaded = true;
    }
    BoardView::previewStone(window, startX, startY, step, blackStoneTex, whiteStoneTex, view);
}

void BoardView::renderBottomControls(const ImVec2& windowSize, GameView& view) {
    float btnWidth = 200.0f;
    float btnHeight = 35.0f;

    ImGui::SetNextWindowPos(ImVec2((windowSize.x - btnWidth) * 0.5f, windowSize.y - 65.0f));
    ImGui::SetNextWindowSize(ImVec2(btnWidth, btnHeight + 10.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar 
                           | ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoBackground
                           | ImGuiWindowFlags_NoScrollbar
                           | ImGuiWindowFlags_NoScrollWithMouse;
 
    ImGui::Begin("BottomControls", nullptr, flags);
    if (ImGui::Button("Return to Menu", ImVec2(btnWidth, btnHeight))) {
        view.gameStarted = false;
    }
    ImGui::End();
}

void BoardView::render(sf::RenderWindow& window, GameInfo& gameInfo, GameService& gameService) {
    ImVec2 windowSize(static_cast<float>(window.getSize().x), 
                      static_cast<float>(window.getSize().y));

    // 1. Dessiner le Goban au centre (taille carrée ~650x650)
    renderBoard(window, ImVec2(windowSize.x * 0.5f, windowSize.y * 0.44f), 640.0f, gameInfo.gameView);

    // 2. Panneaux latéraux ImGui
    renderPlayerPanel("BlackPlayerPanel", gameInfo.p1Info, true, windowSize, gameInfo.gameView.tipsEnable, gameInfo);
    renderPlayerPanel("WhitePlayerPanel", gameInfo.p2Info, false, windowSize, gameInfo.gameView.tipsEnable, gameInfo);

    // 3. Bouton retour au menu en bas
    renderBottomControls(windowSize, gameInfo.gameView);

    if (m_showVictoryPopup && m_winnerInfo != nullptr) {
        renderVictoryPopup(*m_winnerInfo, m_victoryReason, gameInfo, gameService);
    }
}