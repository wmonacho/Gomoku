#pragma once

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <string>
#include "../app/Dto.hpp"
#include <cmath>

class BoardView {
public:
    BoardView();
    ~BoardView() = default;

    void render(sf::RenderWindow& window, GameInfo& gameInfo);
    void handleInput(const sf::Event& event, const sf::RenderWindow& window, GameView& view);

private:
    void renderPlayerPanel(const std::string& panelId, const PlayerInfo& player, 
                           bool isLeft, const ImVec2& windowSize, bool tipsEnable);
    void renderBoard(sf::RenderWindow& window, const ImVec2& centerPos, float boardSize, const GameView& view);
    void renderBottomControls(const ImVec2& windowSize, GameView& view);
    sf::Font m_font;
    bool m_fontLoaded = false;
};