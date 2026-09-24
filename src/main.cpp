#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h> // Si nécessaire, toujours APRES imgui-SFML.h

int main() {
    sf::RenderWindow window(sf::VideoMode(1400, 800), "Gomoku");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window))
        return -1;

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed)
                window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        // Fenêtre ImGui de test
        ImGui::Begin("Parametres IA");
        ImGui::Text("Tour : Joueur 1 (Noir)");
        if (ImGui::Button("Test Clic")) {
            // Action test
        }
        ImGui::End();

        window.clear(sf::Color(50, 50, 50));
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}