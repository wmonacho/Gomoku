#pragma once

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <string>
#include "../app/Dto.hpp"
#include <cmath>
#include "app/GameService.hpp"

class BoardView {
public:
    BoardView();
    ~BoardView() = default;

    void render(sf::RenderWindow& window, GameInfo& gameInfo, GameService& gameService);
    void handleInput(const sf::Event& event, const sf::RenderWindow& window, GameInfo& gameInfo, GameService& gameService);

private:
    void renderPlayerPanel(const std::string& panelId, const PlayerInfo& player, 
                           bool isLeft, const ImVec2& windowSize, bool tipsEnable, GameInfo& gameInfo);
    void renderBoard(sf::RenderWindow& window, const ImVec2& centerPos, float boardSize, const GameView& view);
    void renderBottomControls(const ImVec2& windowSize, GameView& view);
    void renderVictoryPopup(const PlayerInfo& winner, char reason, GameInfo& gameInfo); 

    sf::Font m_font;
    bool m_fontLoaded = false;

    bool m_showVictoryPopup = false;
    const PlayerInfo* m_winnerInfo = nullptr;
    char m_victoryReason = ' ';
};