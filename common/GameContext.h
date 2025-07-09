#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include <SDL2/SDL.h>
#include <string>
#include <chrono>

// Enum for screen/state control
enum GameState {
    MENU,
    CONTINUE,
    FLOOR1,
    FLOOR2,
    FLOOR3,
    EXIT
};

struct GameContext {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::string playerName;
    bool startGameRequested = false;
    GameState nextState = MENU;

    // ⏱ Timer start point
    std::chrono::steady_clock::time_point startTime;
};

#endif // GAME_CONTEXT_H
