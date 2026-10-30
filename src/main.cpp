#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include "ui/mainMenu.hpp"
#include "ui/BoardView.hpp"

ImFont* chargeFont() {
    ImGuiIO& io = ImGui::GetIO();

    ImFont* bodyFont = io.Fonts->AddFontFromFileTTF("assets/fonts/Monocraft.ttf", 18.0f);
    if (!bodyFont) {
        std::cerr << "Error: Failed to load font 'assets/fonts/Monocraft.ttf' (18px)" << std::endl;
        return nullptr;
    }
    io.FontDefault = bodyFont;

    ImFont* titleFont = io.Fonts->AddFontFromFileTTF("assets/fonts/Monocraft.ttf", 150.0f);
    if (!titleFont) {
        std::cerr << "Error: Failed to load font 'assets/fonts/Monocraft.ttf' (150px)" << std::endl;
        return nullptr;
    }

    if (!ImGui::SFML::UpdateFontTexture()) {
        std::cerr << "Error: Failed to update ImGui-SFML font texture" << std::endl;
        return nullptr;
    }
    return titleFont;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1400, 800), "Gomoku");
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window)) {
        return -1;
    }

    MainMenu menu;
    BoardView boardView;
    GameInfo gameInfo;
    if (!menu.loadBackgroundFrames(60)) {
        return -1;
    }

    ImFont* titleFont = chargeFont();
    if (!titleFont) {
        std::cerr << "Fatal Error: Failed to initialize fonts. Aborting." << std::endl;
        ImGui::SFML::Shutdown();
        return -1;
    }
    menu.setTitleFont(titleFont);


    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed)
                window.close();
            if (gameInfo.gameView.gameStarted) {
                boardView.handleInput(event, window, gameInfo);
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        window.clear(sf::Color(30, 30, 30));

        if (!gameInfo.gameView.gameStarted) {
            menu.render(window, gameInfo);
        } else {
            boardView.render(window, gameInfo);
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}