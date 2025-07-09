#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include "GameContext.h"
#include "game_state.h"
#include "button.h" // <-- Add this line

// Remove the Button struct definition from here!

GameState runMenu(GameContext& ctx);

#endif // MENU_H