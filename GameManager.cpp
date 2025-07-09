#include "GameManager.h"
#include "common/game_state.h"
#include "common/GameContext.h"
#include "UI/menu.h"
#include "UI/input.h"
#include "UI/leaderboard.h"
#include "floors/floor1/floor1.h"
#include "floors/floor2/floor2.h"
#include "floors/floor3/floor3.h"
#include <iostream>
#include <SDL2/SDL_ttf.h>

GameManager::GameManager() {}

void GameManager::run(GameContext& context) {
    GameState menuResult = runMenu(context);
    context.nextState = menuResult;  // Record user choice
    if (menuResult == EXIT) return;

    setCurrentFloor(1); // Start game at Floor 1

    while (true) {
        // 🛑 Check if player hit "Quit" from any floor
        if (context.nextState == MENU) {
            menuResult = runMenu(context);
            context.nextState = menuResult;
            if (menuResult == EXIT) return;
            setCurrentFloor(1);
            continue;
        } else if (context.nextState == EXIT) {
            return;
        }

        int currentFloor = getCurrentFloor();

        if (currentFloor == 1) {
            runFloor1(context); // May set context.nextState = MENU
        } 
        else if (currentFloor == 2) {
            runFloor2(context);
        }
        else if (currentFloor == 3) {
            runFloor3(context);
            context.nextState = MENU;
        } 
        else {
            std::cout << "Unknown floor. Exiting...\n";
            break;
        }
    }


}


