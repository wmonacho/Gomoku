#pragma once

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <string>

struct PlayerInfo {
    std::string name;
    bool isAI = false;
    int capturedStones = 0;
    float lastMoveTime = 0.0f; // En secondes
    std::string lastMoveCoord = "-";
};

class BoardView {
public:
    BoardView();
    ~BoardView() = default;

    void render(sf::RenderWindow& window);

    bool shouldReturnToMenu() const { return m_returnToMenu; }
    void resetReturnToMenu() { m_returnToMenu = false; }

private:
    void renderPlayerPanel(const std::string& panelId, const PlayerInfo& player, 
                           bool isLeft, const ImVec2& windowSize);
    void renderBoard(sf::RenderWindow& window, const ImVec2& centerPos, float boardSize);
    void renderBottomControls(const ImVec2& windowSize);

    bool m_returnToMenu = false;

    // Données de simulation pour l'affichage
    PlayerInfo m_blackPlayer{"Player 1 (Black)", false, 2, 0.0f, "K10"};
    PlayerInfo m_whitePlayer{"Player 2 (White / AI)", true, 0, 1.24f, "L11"};
};