#include "mainMenu.hpp"

MainMenu::MainMenu() = default;

MainMenu::~MainMenu() {}

bool MainMenu::loadBackgroundFrames(int totalFrames) {
    m_bgTextures.clear();
    m_bgTextures.resize(totalFrames);

    for (int i = 1; i <= totalFrames; ++i) { 
        std::ostringstream pathStream;
        pathStream << "assets/images/ezgif-frame-" 
                   << std::setfill('0') << std::setw(3) << i 
                   << ".jpg";

        if (!m_bgTextures[i - 1].loadFromFile(pathStream.str())) {
            std::cerr << "Error: Failed to load image " << pathStream.str() << std::endl;
            return false;
        }   
        m_bgTextures[i - 1].setSmooth(true);
    }

    if (!m_bgTextures.empty()) {
        m_bgSprite.setTexture(m_bgTextures[0]);
    }
    return true;
}

void MainMenu::setTitleFont(ImFont* font) {
    if (!font) {
        std::cerr << "Warning: setTitleFont received a null pointer." << std::endl;
        return;
    }
    m_titleFont = font;
}

void MainMenu::renderBackground(sf::RenderWindow& window) {
    if (m_bgTextures.empty()) {
        return;
    }

    if (m_animClock.getElapsedTime().asSeconds() >= m_frameDuration) {
        m_currentFrame += m_frameDirection;

        if (m_currentFrame >= static_cast<int>(m_bgTextures.size())) {
            m_currentFrame = static_cast<int>(m_bgTextures.size()) - 2;
            m_frameDirection = -1;
        } else if (m_currentFrame < 0) {
            m_currentFrame = 1;
            m_frameDirection = 1;
        }

        m_bgSprite.setTexture(m_bgTextures[static_cast<size_t>(m_currentFrame)], true);
        m_animClock.restart();
    }

    float scaleX = static_cast<float>(window.getSize().x) / m_bgTextures[static_cast<size_t>(m_currentFrame)].getSize().x;
    float scaleY = static_cast<float>(window.getSize().y) / m_bgTextures[static_cast<size_t>(m_currentFrame)].getSize().y;
    m_bgSprite.setScale(scaleX, scaleY);

    window.draw(m_bgSprite);
}

void MainMenu::renderHeader(const ImVec2& windowSize) {
    if (m_titleFont != nullptr) {
        ImGui::PushFont(m_titleFont);
    }

    const char* gameName = "GOMOKU";
    float titleWidth = ImGui::CalcTextSize(gameName).x;
    ImGui::SetCursorPosX((windowSize.x - titleWidth) * 0.5f);
    ImGui::SetCursorPosY(windowSize.y * 0.20f);
    ImGui::TextColored(ImVec4(0.95f, 0.77f, 0.25f, 1.0f), "%s", gameName);

    if (m_titleFont != nullptr) {
        ImGui::PopFont();
    }

    const char* authorText = "Made by wmonacho & lbattest";
    float authorWidth = ImGui::CalcTextSize(authorText).x;
    ImGui::SetCursorPosX((windowSize.x - authorWidth) * 0.5f);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", authorText);

    for (int i = 0; i < 7; ++i) {
        ImGui::Spacing();
    }
    ImGui::Separator();
}

void MainMenu::clearInfo(GameInfo& gameInfo) {
    gameInfo.gameView.board.fill(Cell::Empty);
    gameInfo.p1Info.capturedStones = 0;
    gameInfo.p1Info.lastMoveTime = 0.0f;
    gameInfo.p1Info.lastMoveCoord = "-";

    gameInfo.p2Info.capturedStones = 0;
    gameInfo.p2Info.lastMoveTime = 0.0f;
    gameInfo.p2Info.lastMoveCoord = "-";

    gameInfo.gameView.gameStarted = true;
}

void MainMenu::renderModeButtons(const ImVec2& windowSize, GameInfo& gameInfo) {
    ImGuiStyle& style = ImGui::GetStyle();

    const char* instructionText = "Choose a game mode:";
    float instructionWidth = ImGui::CalcTextSize(instructionText).x;
    ImGui::SetCursorPosX((windowSize.x - instructionWidth) * 0.5f);
    ImGui::Text("%s", instructionText);

    ImGui::Spacing();
    ImGui::Spacing();

    float buttonWidth  = 180.0f;
    float buttonHeight = 40.0f;
    float spacing      = style.ItemSpacing.x;
    float totalButtonsWidth = (buttonWidth * 3.0f) + (spacing * 2.0f);

    ImGui::SetCursorPosX((windowSize.x - totalButtonsWidth) * 0.5f);

    if (ImGui::Button("Human vs AI", ImVec2(buttonWidth, buttonHeight))) {
        m_selectedMode = "Human vs AI";
        gameInfo.p1Info.isAI = false;
        gameInfo.p2Info.isAI = true;
        MainMenu::clearInfo(gameInfo);
    }

    ImGui::SameLine();

    if (ImGui::Button("Human vs Human", ImVec2(buttonWidth, buttonHeight))) {
        m_selectedMode = "Human vs Human";
        gameInfo.p1Info.isAI = false;
        gameInfo.p2Info.isAI = false;
        MainMenu::clearInfo(gameInfo);
    }

    ImGui::SameLine();

    if (ImGui::Button("AI vs AI", ImVec2(buttonWidth, buttonHeight))) {
        m_selectedMode = "AI vs AI";
        gameInfo.p1Info.isAI = true;
        gameInfo.p2Info.isAI = true;
        MainMenu::clearInfo(gameInfo);
    }

    ImGui::Spacing();
    ImGui::Spacing();
}

void MainMenu::renderOptions(const ImVec2& windowSize, GameInfo& gameInfo) {
    ImGuiStyle& style = ImGui::GetStyle();

    const char* checkboxLabel = "Activate tips in game";
    float checkboxTotalWidth = ImGui::GetFrameHeight() + style.ItemInnerSpacing.x + ImGui::CalcTextSize(checkboxLabel).x;

    ImGui::SetCursorPosX((windowSize.x - checkboxTotalWidth) * 0.5f);
    if (ImGui::Checkbox("Activate tips in game", &gameInfo.gameView.tipsEnable)) {
        std::cout << "Tips: " << (gameInfo.gameView.tipsEnable ? "ON" : "OFF") << std::endl;
    }

    ImGui::Spacing();
    ImGui::Separator();
}

void MainMenu::render(sf::RenderWindow& window, GameInfo& gameInfo) {
    // 1. Fond SFML
    renderBackground(window);

    // Window ImGui fullscreen
    ImGui::SetNextWindowBgAlpha(0.4f);
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window.getSize().x),
                                    static_cast<float>(window.getSize().y)));

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar
                                 | ImGuiWindowFlags_NoResize
                                 | ImGuiWindowFlags_NoMove
                                 | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("FullscreenMenu", nullptr, windowFlags);

    ImVec2 windowSize = ImGui::GetWindowSize();

    renderHeader(windowSize);
    renderModeButtons(windowSize, gameInfo);
    renderOptions(windowSize, gameInfo);

    ImGui::End();
}

std::string MainMenu::getSelectedMode() const {
    return m_selectedMode;
}