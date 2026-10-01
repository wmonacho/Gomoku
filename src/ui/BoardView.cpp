#include "ui/BoardView.hpp"
#include <iostream>

BoardView::BoardView() : m_returnToMenu(false) {}

void BoardView::renderPlayerPanel(const std::string& panelId, const PlayerInfo& player, 
                                 bool isLeft, const ImVec2& windowSize) {
    float panelWidth = 280.0f;
    float panelHeight = windowSize.y - 40.0f;
    float posX = isLeft ? 20.0f : (windowSize.x - panelWidth - 20.0f);
    float posY = 20.0f;

    ImGui::SetNextWindowPos(ImVec2(posX, posY));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin(panelId.c_str(), nullptr, flags);

    // Titre / Joueur
    ImGui::TextColored(isLeft ? ImVec4(0.8f, 0.8f, 0.8f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                       "%s", player.name.c_str());
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

    // Bouton de forfait
    if (ImGui::Button("Forfeit", ImVec2(-1.0f, 35.0f))) {
        std::cout << player.name << " forfeited the match." << std::endl;
    }

    ImGui::End();
}

void BoardView::renderBoard(sf::RenderWindow& window, const ImVec2& centerPos, float boardSize) {
    // 1. Fond en bois du plateau de Gomoku (Goban)
    sf::RectangleShape goban(sf::Vector2f(boardSize, boardSize));
    goban.setOrigin(boardSize * 0.5f, boardSize * 0.5f);
    goban.setPosition(centerPos.x, centerPos.y);
    goban.setFillColor(sf::Color(220, 179, 92)); // Teinte bois chaud
    goban.setOutlineThickness(3.0f);
    goban.setOutlineColor(sf::Color(40, 25, 10));
    window.draw(goban);

    // 2. Grille 19x19
    float startX = centerPos.x - (boardSize * 0.5f) + 20.0f;
    float startY = centerPos.y - (boardSize * 0.5f) + 20.0f;
    float step = (boardSize - 40.0f) / 18.0f;

    for (int i = 0; i < 19; ++i) {
        // Lignes horizontales
        sf::Vertex hLine[] = {
            sf::Vertex(sf::Vector2f(startX, startY + i * step), sf::Color::Black),
            sf::Vertex(sf::Vector2f(startX + 18 * step, startY + i * step), sf::Color::Black)
        };
        window.draw(hLine, 2, sf::Lines);

        // Lignes verticales
        sf::Vertex vLine[] = {
            sf::Vertex(sf::Vector2f(startX + i * step, startY), sf::Color::Black),
            sf::Vertex(sf::Vector2f(startX + i * step, startY + 18 * step), sf::Color::Black)
        };
        window.draw(vLine, 2, sf::Lines);
    }
}

void BoardView::renderBottomControls(const ImVec2& windowSize) {
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
        m_returnToMenu = true;
    }
    ImGui::End();
}

void BoardView::render(sf::RenderWindow& window) {
    ImVec2 windowSize(static_cast<float>(window.getSize().x), 
                      static_cast<float>(window.getSize().y));

    // 1. Dessiner le Goban au centre (taille carrée ~650x650)
    renderBoard(window, ImVec2(windowSize.x * 0.5f, windowSize.y * 0.44f), 640.0f);

    // 2. Panneaux latéraux ImGui
    renderPlayerPanel("BlackPlayerPanel", m_blackPlayer, true, windowSize);
    renderPlayerPanel("WhitePlayerPanel", m_whitePlayer, false, windowSize);

    // 3. Bouton retour au menu en bas
    renderBottomControls(windowSize);
}