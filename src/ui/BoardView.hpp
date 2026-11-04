#pragma once

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <string>
#include "../app/Dto.hpp"
// #include <cmath>
#include "app/GameService.hpp"

struct CaptureAnim {
    Pos pos;             // Coordonnées de la pièce (x, y)
    Cell color;          // Black ou White
    float elapsed = 0.0f;
    float duration = 0.25f; // Durée en secondes (250 ms)
};

class BoardView {
public:
    BoardView();
    ~BoardView() = default;

    void render(sf::RenderWindow& window, GameInfo& gameInfo, GameService& gameService);
    void handleInput(const sf::Event& event, const sf::RenderWindow& window, GameInfo& gameInfo, GameService& gameService);
    void triggerCaptureAnimations(const std::vector<Pos>& captures, Cell capturedColor);
private:
    void renderPlayerPanel(const std::string& panelId, const PlayerInfo& player, 
                           bool isLeft, const ImVec2& windowSize, bool tipsEnable, GameInfo& gameInfo);
    void renderBoard(sf::RenderWindow& window, const ImVec2& centerPos, float boardSize, const GameView& view);
    void renderBottomControls(const ImVec2& windowSize, GameView& view);
    void renderVictoryPopup(const PlayerInfo& winner, char reason, GameInfo& gameInfo, GameService& gameService);
    void checkMove(MoveResult result, Pos targetPos, GameInfo& gameInfo, GameService& gameService);
    void infoPopup(const PlayerInfo& winner, std::string reasonStr, GameInfo& gameInfo, GameService& gameService);
    // void board(sf::RenderWindow& window, const ImVec2& centerPos, float boardSize, float startX, float startY, float step);
    void renderNumber(sf::RenderWindow& window, float startX, float startY, float step);
    void previewStone(sf::RenderWindow& window, float startX, float startY, float step, sf::Texture blackStoneTex, sf::Texture whiteStoneTex, const GameView& view);
    void loadTextures(sf::Texture& blackStoneTex, sf::Texture& whiteStoneTex);
    void captureStones(sf::RenderWindow& window, float startX, float startY, float step);

    sf::Font m_font;
    bool m_fontLoaded = false;

    bool m_showVictoryPopup = false;
    const PlayerInfo* m_winnerInfo = nullptr;
    char m_victoryReason = ' ';

    std::vector<CaptureAnim> m_captureAnims;
};