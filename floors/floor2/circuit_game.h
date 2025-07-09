#ifndef CIRCUIT_GAME_H
#define CIRCUIT_GAME_H

#include <SDL2/SDL.h>
#include "../../common/GameContext.h"

// Run the circuit game using the shared SDL_Renderer.
// Should be launched in a thread from floor2.cpp.
void runCircuitGame(SDL_Renderer* sharedRenderer,GameContext& ctx);

#endif // CIRCUIT_GAME_H
