// circuit_game.cpp
#include "circuit_game.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "../../common/GameContext.h"


const int WIN_W = 800, WIN_H = 600;
const int COMP_COUNT = 6;
const SDL_Point targetSlots[COMP_COUNT] = {
    {124, 457}, {530, 178}, {534, 282}, {433, 494}, {536, 452}, {125, 305}
};
const char* fileNames[COMP_COUNT] = {
    "assets/images/battery.png",
    "assets/images/resistor.png",
    "assets/images/capacitor.png",
    "assets/images/diode.png",
    "assets/images/voltmeter.png",
    "assets/images/ammeter.png"
};

bool isNear(int x1, int y1, int x2, int y2, int range = 40) {
    return (std::abs(x1 - x2) < range && std::abs(y1 - y2) < range);
}

void runCircuitGame(SDL_Renderer* ren, GameContext& ctx) {
    TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", 24);
    SDL_Texture* background = IMG_LoadTexture(ren, "assets/images/circuit_background.png");
    SDL_Texture* ledTex = IMG_LoadTexture(ren, "assets/images/led.png");

    SDL_Texture* compTex[COMP_COUNT];
    for (int i = 0; i < COMP_COUNT; ++i) {
        compTex[i] = IMG_LoadTexture(ren, fileNames[i]);
    }

    Mix_Chunk* pickSound    = Mix_LoadWAV("assets/audio/pick.mp3");
    Mix_Chunk* placeSound   = Mix_LoadWAV("assets/audio/place.mp3");
    Mix_Chunk* successSound = Mix_LoadWAV("assets/audio/success_circuit.mp3");
    Mix_Chunk* failSound    = Mix_LoadWAV("assets/audio/fail_circuit.mp3");

    struct Comp { SDL_Rect rect; bool placed; };
    std::vector<Comp> comps(COMP_COUNT);
    for (int i = 0; i < COMP_COUNT; ++i) {
        comps[i].rect = {100 + i * 100, 400, 64, 64};
        comps[i].placed = false;
    }

    bool quit = false, paused = false, solved = false;
    bool dragging = false;
    int dragged = -1, offsetX = 0, offsetY = 0;

    Uint32 startTicks = SDL_GetTicks();
    Uint32 pausedTicks = 0;
    const int TIME_LIMIT = 60;
    std::string unlockMsg;

    SDL_StartTextInput();
    while (!quit && !solved) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { quit = true; break; }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE && !solved) {
                paused = !paused;
                if (paused) pausedTicks = SDL_GetTicks() - startTicks;
                else startTicks = SDL_GetTicks() - pausedTicks;
            }

            if (!paused && !solved) {
                if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    int mx = e.button.x, my = e.button.y;
                    for (int i = 0; i < COMP_COUNT; ++i) {
                        SDL_Rect& r = comps[i].rect;
                        if (mx > r.x && mx < r.x + r.w && my > r.y && my < r.y + r.h) {
                            dragging = true; dragged = i;
                            offsetX = mx - r.x; offsetY = my - r.y;
                            Mix_PlayChannel(-1, pickSound, 0);
                            break;
                        }
                    }
                }
                if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                    if (dragged != -1 && isNear(comps[dragged].rect.x, comps[dragged].rect.y,
                                                targetSlots[dragged].x, targetSlots[dragged].y)) {
                        Mix_PlayChannel(-1, placeSound, 0);
                    }
                    dragging = false; dragged = -1;
                }
                if (e.type == SDL_MOUSEMOTION && dragging && dragged != -1) {
                    comps[dragged].rect.x = e.motion.x - offsetX;
                    comps[dragged].rect.y = e.motion.y - offsetY;
                }
            }
        }

        Uint32 now = SDL_GetTicks();
        int secLeft;
        if (paused || solved) secLeft = TIME_LIMIT - (int)(pausedTicks / 1000);
        else {
            pausedTicks = now - startTicks;
            secLeft = TIME_LIMIT - (int)(pausedTicks / 1000);
        }

        if (!paused && !solved && secLeft <= 0) {
            solved = true; unlockMsg = "Time's up! Try again.";
            Mix_PlayChannel(-1, failSound, 0);
        }

        if (!solved) {
            bool allPlaced = true;
            for (int i = 0; i < COMP_COUNT; ++i) {
                if (isNear(comps[i].rect.x, comps[i].rect.y,
                           targetSlots[i].x, targetSlots[i].y)) comps[i].placed = true;
                else { comps[i].placed = false; allPlaced = false; }
            }
            if (allPlaced) {
                solved = true;
                int keyNum = 1000 + std::rand() % 9000;
                unlockMsg = "Puzzle Solved! Unlock Key: " + std::to_string(keyNum);
                Mix_PlayChannel(-1, successSound, 0);
            }
        }

        SDL_SetRenderDrawColor(ren, 20, 20, 20, 255);
        SDL_RenderClear(ren);

        if (background) {
            SDL_Rect bgRect = {0, 0, WIN_W, WIN_H};
            SDL_RenderCopy(ren, background, NULL, &bgRect);
        }

        if (ledTex) {
            SDL_Rect lr = {378, 43, 60, 60};
            SDL_SetTextureColorMod(ledTex, solved ? 0 : 100, solved ? 255 : 100, solved ? 0 : 100);
            SDL_RenderCopy(ren, ledTex, NULL, &lr);
        }

        for (int i = 0; i < COMP_COUNT; ++i) {
            if (compTex[i]) SDL_RenderCopy(ren, compTex[i], NULL, &comps[i].rect);
        }

        if (font) {
            std::stringstream ss;
            ss << "Time Left: " << (secLeft > 0 ? secLeft : 0) << "s";
            SDL_Surface* ts = TTF_RenderText_Blended(font, ss.str().c_str(), {0, 0, 0, 255});
            if (ts) {
                SDL_Texture* tt = SDL_CreateTextureFromSurface(ren, ts);
                SDL_Rect tr = {10, 10, ts->w, ts->h};
                SDL_RenderCopy(ren, tt, NULL, &tr);
                SDL_FreeSurface(ts);
                SDL_DestroyTexture(tt);
            }
        }

        if (solved && font) {
            SDL_Surface* rs = TTF_RenderText_Blended(font, unlockMsg.c_str(), {255, 255, 255, 255});
            if (rs) {
                SDL_Texture* rt = SDL_CreateTextureFromSurface(ren, rs);
                SDL_Rect rr = {150, 500, rs->w, rs->h};
                SDL_RenderCopy(ren, rt, NULL, &rr);
                SDL_FreeSurface(rs);
                SDL_DestroyTexture(rt);
            }
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
    if (solved) ctx.nextState = FLOOR1;
    Mix_FreeChunk(pickSound);
    Mix_FreeChunk(placeSound);
    Mix_FreeChunk(successSound);
    Mix_FreeChunk(failSound);
    TTF_CloseFont(font);
    for (int i = 0; i < COMP_COUNT; ++i) SDL_DestroyTexture(compTex[i]);
    SDL_DestroyTexture(ledTex);
    SDL_DestroyTexture(background);
    SDL_StopTextInput();
    return;
}