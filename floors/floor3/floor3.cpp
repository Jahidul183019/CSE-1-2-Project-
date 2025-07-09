#include "floor3.h"
#include "space_shooter.h"
#include "monster_game.h"
#include "../../common/game_state.h"
#include "../../common/utils.h"
#include "../../common/GameContext.h"
#include "../../UI/leaderboard.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <algorithm>
#include <string>
#include "../../common/player.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;
static int WORLD_WIDTH = 1600;
static int WORLD_HEIGHT = 1200;

static SDL_Texture *backgroundTexture = nullptr;
static SDL_Texture *playerTexture = nullptr;
static Mix_Chunk *correctSound = nullptr;

static SDL_Rect camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

static SDL_Rect door1 = {70, 75, 105, 162};  // Empty
static SDL_Rect door2 = {255, 75, 105, 162}; // Space Shooter
static SDL_Rect door3 = {445, 75, 105, 162}; // Empty
static SDL_Rect door4 = {665, 75, 105, 162}; // Monster Game
static SDL_Rect quitBtn = {20, 20, 100, 40}; // Quit button on screen

static bool shooterWon = false;

Mix_Chunk* moveSound = nullptr;

static bool loadMedia(SDL_Renderer *renderer) {
    backgroundTexture = IMG_LoadTexture(renderer, "assets/images/floor3.png");
    playerTexture = IMG_LoadTexture(renderer, "assets/images/player.png");
    correctSound = Mix_LoadWAV("assets/audio/correct.wav");
    moveSound = Mix_LoadWAV("assets/audio/robot.mp3");  // Load the move sound

    if (!backgroundTexture || !playerTexture || !correctSound || !moveSound) {
        std::cerr << "Media loading failed: " << IMG_GetError() << " / " << Mix_GetError() << std::endl;
        return false;
    }

    SDL_QueryTexture(backgroundTexture, nullptr, nullptr, &WORLD_WIDTH, &WORLD_HEIGHT);
    return true;
}


static bool canMove(int dx, int dy)
{
    SDL_Rect temp = player;
    temp.x += dx;
    temp.y += dy;

    return (temp.x >= 0 && temp.y >= 0 &&
            temp.x + temp.w <= WORLD_WIDTH &&
            temp.y + temp.h <= WORLD_HEIGHT);
}

static void handleInput(SDL_Event &e) {
    if (e.type == SDL_KEYDOWN) {
        int dx = 0, dy = 0;
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:
                dx = -10;
                break;
            case SDLK_RIGHT:
                dx = +10;
                break;
            case SDLK_UP:
                dy = -10;
                break;
            case SDLK_DOWN:
                dy = +10;
                break;
        }
        if (canMove(dx, dy)) {
            player.x = std::clamp(player.x + dx, 0, WORLD_WIDTH - player.w);
            player.y = std::clamp(player.y + dy, 0, WORLD_HEIGHT - player.h);

            // Play the robot movement sound when the player moves
            if (moveSound) {
                Mix_PlayChannel(-1, moveSound, 0);
            }
        }
    }
}


static void updateCamera()
{
    camera.x = player.x + player.w / 2 - camera.w / 2;
    camera.y = player.y + player.h / 2 - camera.h / 2;

    camera.x = std::clamp(camera.x, 0, WORLD_WIDTH - camera.w);
    camera.y = std::clamp(camera.y, 0, WORLD_HEIGHT - camera.h);
}

static void showMessage(SDL_Renderer *renderer, const std::string &text)
{
    TTF_Font *font = TTF_OpenFont("assets/fonts/arial.ttf", 36);
    if (!font)
        return;
    SDL_Color color = {255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {250, 250, 300, 100};

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_RenderPresent(renderer);

    SDL_Delay(1500);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}

static void handleClick(int mx, int my, SDL_Renderer *renderer, bool &quit, GameContext &ctx, bool &monsterPlayed)
{
    if (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w &&
        my >= quitBtn.y && my <= quitBtn.y + quitBtn.h)
    {
        ctx.nextState = MENU;
        quit = true;
        return;
    }

    SDL_Rect click = {mx + camera.x, my + camera.y, 1, 1};

    if (SDL_HasIntersection(&click, &door1))
    {
        showMessage(renderer, "There is nothing.");
        return;
    }

    if (SDL_HasIntersection(&click, &door2))
    {
        Mix_PlayChannel(-1, correctSound, 0);
        showMessage(renderer, "KILL THE ENEMIES \n FACE THE BOSS");
        shooterWon = runSpaceShooterGame(renderer);
        return;
    }

    if (SDL_HasIntersection(&click, &door3))
    {
        showMessage(renderer, "There is nothing.");
        return;
    }

    if (SDL_HasIntersection(&click, &door4))
    {
        if (!shooterWon)
        {
            showMessage(renderer, "LOCKED. KILLED ALL ENEMIES ?");
            return;
        }
        Mix_PlayChannel(-1, correctSound, 0);
        showMessage(renderer, "BEAT THE FINAL BOSS!");
        runMonsterGame(renderer,ctx);
        monsterPlayed = true;
        quit = true;
    }
}

static void render(SDL_Renderer *renderer)
{
    SDL_RenderClear(renderer);
    SDL_Rect src = camera;
    SDL_Rect dst = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, backgroundTexture, &src, &dst);

    SDL_Rect p = {player.x - camera.x, player.y - camera.y, player.w, player.h};
    SDL_RenderCopy(renderer, playerTexture, nullptr, &p);

    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &quitBtn);

    TTF_Font *font = TTF_OpenFont("assets/fonts/arial.ttf", 24);
    if (font)
    {
        SDL_Color color = {255, 255, 255};
        SDL_Rect r;
        SDL_Texture *txt = renderText(renderer, font, "Quit", color, r);
        r.x = quitBtn.x + 20;
        r.y = quitBtn.y + 8;
        SDL_RenderCopy(renderer, txt, nullptr, &r);
        SDL_DestroyTexture(txt);
        TTF_CloseFont(font);
    }

    SDL_RenderPresent(renderer);
}

static void cleanUp() {
    Mix_FreeChunk(correctSound);
    Mix_FreeChunk(moveSound);  // Free the move sound
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(playerTexture);
    correctSound = nullptr;
    moveSound = nullptr;  // Set the move sound to nullptr
    backgroundTexture = nullptr;
    playerTexture = nullptr;
}

void runFloor3(GameContext &ctx)
{
    SDL_Renderer *renderer = ctx.renderer;
    player = {470, 665, 85, 80};

    if (!loadMedia(renderer))
        return;

    bool quit = false;
    bool monsterPlayed = false;
    SDL_Event e;

    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                quit = true;

            handleInput(e);

            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                handleClick(mx, my, renderer, quit, ctx, monsterPlayed);
            }
        }

        updateCamera();
        render(renderer);
    }

    cleanUp();

    // Leaderboard update after Monster Game win
       // Leaderboard update after Monster Game win
   /* if (shooterWon && monsterPlayed)
    {
        TTF_Font *font = TTF_OpenFont("assets/fonts/arial.ttf", 36);
        if (font)
        {
            Leaderboard leaderboard(font);
            leaderboard.loadFromFile("leaderboard.txt");

            using namespace std::chrono;
            float timeSpent = duration_cast<duration<float>>(steady_clock::now() - ctx.startTime).count();

            leaderboard.players.emplace_back(ctx.playerName, timeSpent);

            std::sort(leaderboard.players.begin(), leaderboard.players.end());
            if (leaderboard.players.size() > 5)
                leaderboard.players.resize(5);

            leaderboard.saveToFile("leaderboard.txt");
            TTF_CloseFont(font);
        }

        // ✅ Automatically return to menu after saving leaderboard
        ctx.nextState = MENU;
    }*/
   if(monsterPlayed) ctx.nextState = MENU;
}