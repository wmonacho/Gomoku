#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <imgui.h>
#include <iostream>
#include <imgui-SFML.h>
#include <iomanip>
#include <sstream>
#include <vector>

class MainMenu {
    public:
        MainMenu();
        ~MainMenu();

        void            setTitleFont(ImFont* font);
        void            render(sf::RenderWindow& window);
        bool            isGameStarted() const;
        bool            isTipsEnabled() const;
        std::string     getSelectedMode() const;
        bool            loadBackgroundFrames(int totalFrames = 60);

    private:
        void            renderBackground(sf::RenderWindow& window);
        void            renderHeader(const ImVec2& windowSize);
        void            renderModeButtons(const ImVec2& windowSize);
        void            renderOptions(const ImVec2& windowSize);

        bool                        m_gameStarted = false;
        std::string                 m_selectedMode = "Undefined";
        ImFont*                     m_titleFont = nullptr;
        bool                        m_tipsEnable = false;
        std::vector<sf::Texture>    m_bgTextures{};
        sf::Sprite                  m_bgSprite{};
        sf::Clock                   m_animClock{};
        int                         m_currentFrame = 0;
        float                       m_frameDuration = 0.06f;
        int                         m_frameDirection = 1;
};