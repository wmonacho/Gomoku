#include "app/GameService.hpp"
#include "app/AIDebug.hpp"
#include "core/ai/SearchConfig.hpp"

#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <random>
#include <system_error>

namespace {

    void resetStats(PlayerInfo& player) {
        player.capturedStones = 0;
        player.lastMoveTime = 0.0f;
        player.lastMoveCoord = "-";
    }

    MoveSuggestion noSuggestion() {
        return {NO_MOVE, 0.0, 0, 0, {}, StopReason::MaxDepth, false, 0, {}};
    }
}

// GOMOKU_DEBUG=1 ./Gomoku : chaque coup de l'IA (et chaque suggestion) affiche son raisonnement dans le terminal.
GameService::GameService()
    : mode_(Mode::PvAI), aiDepth_(SearchConfig::DEPTH), debugLog_(std::getenv("GOMOKU_DEBUG") != nullptr) {}

GameService::~GameService() {
    cancelAITurn();
}

void GameService::newGame(GameInfo& gameInfo) {
    assert(gameInfo.p1Info.color != Cell::Empty && gameInfo.p2Info.color != Cell::Empty
           && "newGame() : chaque joueur doit avoir une couleur");
    assert(gameInfo.p1Info.color != gameInfo.p2Info.color && "newGame() : les deux joueurs ont la même couleur");

    // Une recherche de l'ancienne partie ne doit ni jouer dans la nouvelle, ni lire search_ pendant clear().
    cancelAITurn();

    // Premier joueur tiré au hasard. static : la graine n'est tirée qu'une fois, pas à chaque partie.
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.5);

    const bool p1Starts = dist(gen);
    if (p1Starts)
        gameInfo.gameView.currentPlayer = &gameInfo.p1Info;
    else
        gameInfo.gameView.currentPlayer = &gameInfo.p2Info;

    mode_ = gameInfo.selectedMode;
    aiDepth_ = SearchConfig::depthOf(gameInfo.difficulty);
    aiPlays_ = {false, false};
    for (const PlayerInfo* player : {&gameInfo.p1Info, &gameInfo.p2Info})
        aiPlays_[static_cast<int>(player->color) - 1] = player->isAI;
    game_.reset(gameInfo.gameView.currentPlayer->color);
    search_.clear();

    resetStats(gameInfo.p1Info);
    resetStats(gameInfo.p2Info);

    const bool tipsEnable = gameInfo.gameView.tipsEnable;
    gameInfo.gameView.board.fill(Cell::Empty);
    gameInfo.gameView.winner.reset();
    gameInfo.gameView.winReason.reset();

    gameInfo.gameView.tipsEnable = tipsEnable;
    gameInfo.gameView.gameStarted = true;
}

MoveResult GameService::playMove(Pos pos) {
    if (isAIThinking())
        return {false, "The AI is thinking", {}};
    return game_.play(pos);
}

MoveSuggestion GameService::suggestMove() {
    if (game_.isOver() || isAIThinking())
        return noSuggestion();
    try {
        const MoveSuggestion suggestion = search_.findBestMove(game_, SearchConfig::DEPTH, SearchConfig::TIME_LIMIT_MS);
        if (debugLog_)
            std::cout << "[IA] suggestion pour le joueur courant\n" << AIDebug::describe(suggestion) << std::flush;
        return suggestion;
    } catch (const std::exception&) {
        // Mémoire épuisée en pleine recherche : game_ a peut-être gardé des coups d'essai, on ne s'y fie plus.
        return noSuggestion();
    }
}

GameView GameService::currentView() const {
    GameView view;
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            view.board[y * BOARD_SIZE + x] = game_.board().at({x, y});

    view.winner = game_.winner();
    view.winReason = game_.endReason();
    return view;
}

bool GameService::isLegalMove(Pos pos) const {
    return game_.isLegalMove(pos);
}

bool GameService::isAITurn() const {
    return !game_.isOver() && aiPlays_[static_cast<int>(game_.currentPlayer()) - 1];
}

// Vrai tant que le coup n'a pas été récupéré par finishAITurn, même si la recherche a déjà fini.
bool GameService::isAIThinking() const {
    return pendingSearch_.valid();
}

void GameService::startAITurn() {
    if (!isAITurn() || isAIThinking())
        return;

    stopSearch_ = false;
    thinkingSince_ = Clock::now();
    // La recherche joue et annule des coups : elle travaille sur sa copie, game_ reste lisible pendant ce temps.
    auto search = [this, game = game_, depth = aiDepth_]() mutable {
        return search_.findBestMove(game, depth, SearchConfig::TIME_LIMIT_MS, &stopSearch_);
    };
    try {
        pendingSearch_ = std::async(std::launch::async, search);
    } catch (const std::system_error&) {
        // Pas de fil disponible : la recherche se fera dans finishAITurn, en bloquant, plutôt que de planter.
        pendingSearch_ = std::async(std::launch::deferred, search);
    }
}

double GameService::aiThinkingMs() const {
    if (!isAIThinking())
        return 0.0;
    return std::chrono::duration<double, std::milli>(Clock::now() - thinkingSince_).count();
}

std::optional<AIMove> GameService::finishAITurn() {
    if (!isAIThinking() || pendingSearch_.wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
        return std::nullopt;

    std::optional<MoveSuggestion> found;
    try {
        found = pendingSearch_.get();
    } catch (const std::exception&) {
        // Mémoire épuisée pendant la recherche : seule la copie est touchée, on joue un coup de secours.
    }
    MoveSuggestion suggestion = found ? *found : fallbackSuggestion();
    MoveResult result = game_.play(suggestion.pos);
    if (!result.isLegal) {
        suggestion = fallbackSuggestion();
        result = game_.play(suggestion.pos);
    }
    if (debugLog_)
        std::cout << AIDebug::describe(suggestion) << std::flush;
    return AIMove{suggestion, result};
}

AIMove GameService::playAITurn() {
    if (!isAITurn())
        return {noSuggestion(), {false, "It is not the AI's turn", {}}};
    startAITurn();
    pendingSearch_.wait();
    return *finishAITurn();
}

void GameService::cancelAITurn() {
    if (!isAIThinking())
        return;
    stopSearch_ = true;
    try {
        pendingSearch_.get();
    } catch (const std::exception&) {
        // Coup abandonné : son éventuelle erreur ne compte plus.
    }
}

// Premier coup légal : ne sert que si la recherche a échoué, pour que l'IA joue quand même.
MoveSuggestion GameService::fallbackSuggestion() const {
    const double elapsedMs = std::chrono::duration<double, std::milli>(Clock::now() - thinkingSince_).count();
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            if (game_.isLegalMove({x, y}))
                return {{x, y}, elapsedMs, 0, 0, {}, StopReason::TimeUp, false, 0, {}};
    return noSuggestion();
}
