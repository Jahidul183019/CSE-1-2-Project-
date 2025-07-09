#include "floor2.h"
#include "../../common/game_state.h"
#include "../../common/utils.h"
#include "../../common/GameContext.h"
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <algorithm>

bool runTetrisGame(SDL_Renderer* renderer);
void runCircuitGame(SDL_Renderer* renderer,GameContext& ctx);
void runProjectionGame(SDL_Renderer* renderer);

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

static SDL_Rect player = {480, 700, 50, 50};
static SDL_Rect camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
static SDL_Texture* backgroundTexture = nullptr;
static SDL_Texture* playerTexture = nullptr;
static Mix_Chunk* correctSound = nullptr;
static Mix_Chunk* moveSfx = nullptr;

static SDL_Rect door1 = {112, 85, 75, 120};
static SDL_Rect door2 = {270, 85, 65, 120};
static SDL_Rect door3 = {475, 85, 75, 120};
static SDL_Rect door4 = {685, 85, 75, 120};
static SDL_Rect obstacles[] = {{285, 265, 450, 370}};
static SDL_Rect quitBtn = {20, 20, 100, 40};

static int WORLD_WIDTH = 1600;
static int WORLD_HEIGHT = 1200;

static bool loadMedia(SDL_Renderer* renderer) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Smooth scaling

    backgroundTexture = IMG_LoadTexture(renderer, "assets/images/floor2.png");
    playerTexture = IMG_LoadTexture(renderer, "assets/images/player.png");
    moveSfx = Mix_LoadWAV("assets/audio/robot.mp3");

    if (!backgroundTexture || !playerTexture || !moveSfx) {
        std::cerr << "Failed to load assets\n";
        return false;
    }

    SDL_QueryTexture(backgroundTexture, NULL, NULL, &WORLD_WIDTH, &WORLD_HEIGHT);
    return true;
}


static bool canMove(int dx, int dy) {
    SDL_Rect temp = player;
    temp.x += dx;
    temp.y += dy;

    if (SDL_HasIntersection(&temp, &obstacles[0])) return false;
    if (SDL_HasIntersection(&temp, &door1)) return false;
    if (SDL_HasIntersection(&temp, &door2)) return false;
    if (SDL_HasIntersection(&temp, &door3)) return false;
    if (SDL_HasIntersection(&temp, &door4)) return false;

    return (temp.x >= 0 && temp.y >= 0 &&
            temp.x + temp.w <= WORLD_WIDTH &&
            temp.y + temp.h <= WORLD_HEIGHT);
}

static void handleInput(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        int dx = 0, dy = 0;
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:  dx = -10; break;
            case SDLK_RIGHT: dx = +10; break;
            case SDLK_UP:    dy = -10; break;
            case SDLK_DOWN:  dy = +10; break;
        }
        if (canMove(dx, dy)) {
            player.x += dx;
            player.y += dy;
            if (moveSfx) Mix_PlayChannel(-1, moveSfx, 0);
        }
    }
}

static void updateCamera() {
    camera.x = player.x + player.w / 2 - camera.w / 2;
    camera.y = player.y + player.h / 2 - camera.h / 2;

    camera.x = std::clamp(camera.x, 0, WORLD_WIDTH - camera.w);
    camera.y = std::clamp(camera.y, 0, WORLD_HEIGHT - camera.h);
}

static void showMessage(SDL_Renderer* renderer, const std::string& text) {
    TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", 36);
    if (!font) return;
    SDL_Color color = {255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {250, 250, 300, 100};

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_RenderPresent(renderer);

    SDL_Delay(1500);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}


static void handleClick(int mx, int my, SDL_Renderer* renderer, bool& quit, GameContext& ctx) {
    SDL_Rect click = {mx + camera.x, my + camera.y, 1, 1};

    if (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w &&
        my >= quitBtn.y && my <= quitBtn.y + quitBtn.h) {
        ctx.nextState = MENU;
        quit = true;
        return;
    }

    if (!isTetrisSolved() && SDL_HasIntersection(&click, &door1)  && player.x > 95 && player.x < 170   ) {
        Mix_PlayChannel(-1, correctSound, 0);
        showMessage(renderer, "Tetris Challenge!");
        if (runTetrisGame(renderer)) {
            setTetrisSolved(true);
            showMessage(renderer, "Tetris Solved!");
        } else {
            showMessage(renderer, "Score < 1000. Returning to Menu...");
            ctx.nextState = MENU;
            quit = true;
        }
    } else if (isTetrisSolved() && !isCircuitSolved() && SDL_HasIntersection(&click, &door2)  && player.x > 250 && player.x < 315   ) {
        Mix_PlayChannel(-1, correctSound, 0);
        showMessage(renderer, "Circuit Challenge!");
        runCircuitGame(renderer,ctx);
        setCircuitSolved(true);
    } else if (isCircuitSolved() && !isProjectionSolved() && SDL_HasIntersection(&click, &door3)  && player.x > 455 && player.x < 530    ) {
        Mix_PlayChannel(-1, correctSound, 0);
        showMessage(renderer, "Projection Challenge!");
        runProjectionGame(renderer);
        setProjectionSolved(true);
    } else if (isProjectionSolved() && SDL_HasIntersection(&click, &door4) && player.x > 665 && player.x < 740   ) {
        Mix_PlayChannel(-1, correctSound, 0);
        showMessage(renderer, "Floor 3 Unlocked!");
        advanceToNextFloor();
        quit = true;
    }
}



static void render(SDL_Renderer* renderer) {
    SDL_RenderClear(renderer);
    SDL_Rect src = camera;
    SDL_Rect dst = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, backgroundTexture, &src, &dst);

    SDL_Rect playerOnScreen = {player.x - camera.x, player.y - camera.y, player.w, player.h};
    SDL_RenderCopy(renderer, playerTexture, nullptr, &playerOnScreen);

    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &quitBtn);
    TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", 24);
    if (font) {
        SDL_Color color = {255, 255, 255};
        SDL_Rect r;
        SDL_Texture* txt = renderText(renderer, font, "Quit", color, r);
        r.x = quitBtn.x + 20;
        r.y = quitBtn.y + 8;
        SDL_RenderCopy(renderer, txt, nullptr, &r);
        SDL_DestroyTexture(txt);
        TTF_CloseFont(font);
    }

    SDL_RenderPresent(renderer);
}

static void cleanUp() {
    if (correctSound) Mix_FreeChunk(correctSound);
    if (moveSfx) Mix_FreeChunk(moveSfx);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(playerTexture);
    backgroundTexture = nullptr;
    playerTexture = nullptr;
    correctSound = nullptr;
    moveSfx = nullptr;
}

void runFloor2(GameContext& ctx) {
    SDL_Renderer* renderer = ctx.renderer;
    player = {480, 700, 50, 50};

    if (!loadMedia(renderer)) return;

    SDL_Event e;
    bool quit = false;

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
                handleClick(mx, my, renderer, quit, ctx);
            }
        }

        updateCamera();
        render(renderer);
    }

    cleanUp();
}