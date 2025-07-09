#include "../../common/game_state.h"
#include "floor1.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <algorithm>
#include "../common/GameContext.h"
#include "../../common/player.h"
#include "puzzle_game.h"
#include "rsa_game.h"
#include "../../common/utils.h"

bool runPuzzleGame(SDL_Renderer* renderer);
void runRSAGame(SDL_Renderer* renderer);

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;
static int WORLD_WIDTH = 1600;
static int WORLD_HEIGHT = 1200;
static bool puzzleSolved = false;

static SDL_Texture* backgroundTexture = nullptr;
static SDL_Texture* playerTexture = nullptr;
static Mix_Chunk* moveSfx = nullptr; // Robot move sound

static SDL_Rect camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

static SDL_Rect obstacles[]  = {{300, 280, 450, 400}};
static SDL_Rect obstacles1[] = {{200, 80, 100, 80}};
static SDL_Rect obstacles2[] = {{450, 80, 100, 80}};
static SDL_Rect obstacles3[] = {{720, 80, 100, 80}};

static int redCount = sizeof(obstacles) / sizeof(obstacles[0]);
static int count1   = sizeof(obstacles1) / sizeof(obstacles1[0]);
static int count2   = sizeof(obstacles2) / sizeof(obstacles2[0]);
static int count3   = sizeof(obstacles3) / sizeof(obstacles3[0]);

static bool loadMedia(SDL_Renderer* renderer) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Smooth scaling

    backgroundTexture = IMG_LoadTexture(renderer, "assets/images/floor1.png");
    playerTexture = IMG_LoadTexture(renderer, "assets/images/player.png");
    moveSfx = Mix_LoadWAV("assets/audio/robot.mp3");

    if (!backgroundTexture || !playerTexture || !moveSfx) {
        std::cerr << "Failed to load assets\n";
        return false;
    }

    SDL_QueryTexture(backgroundTexture, NULL, NULL, &WORLD_WIDTH, &WORLD_HEIGHT);
    return true;
}

static bool canMove(int x, int y) {
    SDL_Rect tempPlayer = player;
    tempPlayer.x += x;
    tempPlayer.y += y;

    for (int i = 0; i < redCount; i++)
        if (SDL_HasIntersection(&tempPlayer, &obstacles[i])) return false;
    for (int i = 0; i < count1; i++)
        if (SDL_HasIntersection(&tempPlayer, &obstacles1[i])) return false;
    for (int i = 0; i < count2; i++)
        if (SDL_HasIntersection(&tempPlayer, &obstacles2[i])) return false;
    for (int i = 0; i < count3; i++)
        if (SDL_HasIntersection(&tempPlayer, &obstacles3[i])) return false;

    return true;
}

static void handleInput(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        bool canMovePlayer = true;
        int dx = 0, dy = 0;
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:  dx = -10; break;
            case SDLK_RIGHT: dx = 10;  break;
            case SDLK_UP:    dy = -10; break;
            case SDLK_DOWN:  dy = 10;  break;
        }
        canMovePlayer = canMove(dx, dy);
        if (canMovePlayer) {
            player.x += dx;
            player.y += dy;
            if (moveSfx) Mix_PlayChannel(-1, moveSfx, 0);
        }
        player.x = std::clamp(player.x, 0, WORLD_WIDTH - player.w);
        player.y = std::clamp(player.y, 0, WORLD_HEIGHT - player.h);
    }
}

static void updateCamera() {
    camera.x = player.x + player.w / 2 - camera.w / 2;
    camera.y = player.y + player.h / 2 - camera.h / 2;
    camera.x = std::clamp(camera.x, 0, WORLD_WIDTH - camera.w);
    camera.y = std::clamp(camera.y, 0, WORLD_HEIGHT - camera.h);
}

static void render(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect quitBtn) {
    SDL_RenderClear(renderer);
    SDL_Rect bgSrcRect = camera;
    SDL_Rect bgDstRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, backgroundTexture, &bgSrcRect, &bgDstRect);

    SDL_Rect playerOnScreen = {player.x - camera.x, player.y - camera.y, player.w, player.h};
    SDL_RenderCopy(renderer, playerTexture, NULL, &playerOnScreen);

    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &quitBtn);
    SDL_Color white = {255,255,255};
    SDL_Rect r;
    SDL_Texture* txt = renderText(renderer, font, "Quit", white, r,0);
    r.x = quitBtn.x + 20;
    r.y = quitBtn.y + 8;
    SDL_RenderCopy(renderer, txt, nullptr, &r);
    SDL_DestroyTexture(txt);

    SDL_RenderPresent(renderer);
}

static void cleanUp() {
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(playerTexture);
    if (moveSfx) Mix_FreeChunk(moveSfx);
    backgroundTexture = nullptr;
    playerTexture = nullptr;
    moveSfx = nullptr;
}

static void runPuzzle1(SDL_Renderer* renderer) {
    SDL_Color color = {255, 255, 255};
    TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", 36);
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Door Opened!", color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect msgRect = {250, 250, 300, 100};
    SDL_RenderCopy(renderer, texture, nullptr, &msgRect);
    SDL_RenderPresent(renderer);
    SDL_Delay(1500);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);

    if(runPuzzleGame(renderer)) {
        puzzleSolved = true;
    }
}

static void runRSAFromDoor2(SDL_Renderer* renderer) {
    runRSAGame(renderer);  // No changes to logic here
    if (rsaSolved) {  // Check that RSA game was solved
        rsaSolved = true;
    }
}





static void handleClick(int mx, int my, SDL_Renderer* renderer) {
    SDL_Rect clickPoint = {mx + camera.x, my + camera.y, 1, 1};

    // Door 1 (Puzzle Game) - opens only if clicked on obstacles1
    for (int i = 0; i < count1; i++)
        if (SDL_HasIntersection(&clickPoint, &obstacles1[i])  && player.x > 210 && player.x < 296) runPuzzle1(renderer);

    // Door 2 (RSA Game) - opens only if puzzle is solved
    for (int i = 0; i < count2; i++)
        if (puzzleSolved && SDL_HasIntersection(&clickPoint, &obstacles2[i]) && player.x > 445 && player.x < 535 ) runRSAFromDoor2(renderer);

    // Door 3 (Unlocked after RSA is solved)
    for (int i = 0; i < count3; i++)
        if (rsaSolved && SDL_HasIntersection(&clickPoint, &obstacles3[i])   && player.x > 706 && player.x < 793 ) {
            SDL_Color color = {255, 255, 0};
            TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", 36);
            SDL_Surface* surface = TTF_RenderText_Solid(font, "Door 3 Unlocked!", color);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_Rect msgRect = {250, 250, 300, 100};
            SDL_RenderCopy(renderer, texture, nullptr, &msgRect);
            SDL_RenderPresent(renderer);
            SDL_Delay(1500);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
            TTF_CloseFont(font);
            advanceToNextFloor();
        }
}

void runFloor1(GameContext& ctx) {
    SDL_Renderer* renderer = ctx.renderer;
    player = {50, 100, 64, 64};
    if (!loadMedia(renderer)) return;

    SDL_Rect quitBtn = {20, 20, 100, 40};
    TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", 24);
    if (!font) return;

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                SDL_Quit();
                exit(0);
            }

            handleInput(e);

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);

                if (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w &&
                    my >= quitBtn.y && my <= quitBtn.y + quitBtn.h) {
                    ctx.nextState = MENU;
                    quit = true;
                    break;
                }

                handleClick(mx, my, renderer);
                if (getCurrentFloor() != 1) quit = true;
            }
        }

        updateCamera();
        render(renderer, font, quitBtn);
    }

    TTF_CloseFont(font);
    cleanUp();
}