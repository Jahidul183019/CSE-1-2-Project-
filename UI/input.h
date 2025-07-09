#ifndef INPUT_H
#define INPUT_H

#include "GameContext.h" 
#include <string>
#include <SDL2/SDL_ttf.h>

// Use this version only
bool getPlayerName(GameContext& ctx, TTF_Font* font);

#endif // INPUT_H
