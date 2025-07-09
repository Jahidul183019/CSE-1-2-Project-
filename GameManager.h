// GameManager.h
#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "common/GameContext.h"

// ----------------------------------------------------
// GameManager class controls the main flow of the game
// ----------------------------------------------------

class GameManager {
public:
    GameManager();                         // Default constructor
    void run(GameContext& context);        // Starts the game loop and handles floor transitions
};

#endif // GAME_MANAGER_H
