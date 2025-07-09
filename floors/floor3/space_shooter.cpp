#include "space_shooter.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int WIN_SCORE = 300;

struct Bullet {
    SDL_Rect rect;
    int speed = -10;
    Bullet(SDL_Rect r, int s = -10) : rect(r), speed(s) {}
};

struct Enemy {
    SDL_Rect rect;
    std::string label;
    int speed = 1;
};

bool checkCollision(SDL_Rect a, SDL_Rect b) {
    return SDL_HasIntersection(&a, &b);
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;
    SDL_Rect dst = {x, y, 0, 0};
    SDL_QueryTexture(texture, NULL, NULL, &dst.w, &dst.h);
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
}

std::string generateEncryptedCode() {
    std::string code = "Encrypted code: ";
    for (int i = 0; i < 16; ++i) code += char('A' + rand() % 26);
    return code;
}

void showEndScreen(SDL_Renderer* renderer, TTF_Font* font, int score, bool won) {
    SDL_Color white = {255,255,255,255};
    SDL_Color green = {0,255,0,255};
    SDL_Color red   = {255,0,0,255};
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_RenderClear(renderer);
    renderText(renderer, font, "Game Over!", white, 320, 180);
    renderText(renderer, font, "Score: " + std::to_string(score), white, 300, 240);
    if (won) renderText(renderer, font, "You killed all enemies!", green, 220, 300);
    else     renderText(renderer, font, "Try Again!", red, 300, 300);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);
}

bool runSpaceShooterGame(SDL_Renderer* renderer) {
    srand((unsigned)time(NULL));

    Mix_Music* bgm      = Mix_LoadMUS("assets/audio/spaceshooter_background.mp3");
    Mix_Chunk* shootSnd = Mix_LoadWAV("assets/audio/space_shoot.mp3");
    if (bgm) Mix_PlayMusic(bgm, -1);

    TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", 24);
    if (!font) return false;

    SDL_Texture* bgTex = IMG_LoadTexture(renderer, "assets/images/space_background.png");
    SDL_Texture* playerTex = IMG_LoadTexture(renderer, "assets/images/ship1.png");
    SDL_Texture* enemyTex = IMG_LoadTexture(renderer, "assets/images/ship2.png");

    SDL_Rect player = { SCREEN_WIDTH/2 - 25, SCREEN_HEIGHT - 60, 50, 40 };
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::string labels[] = {"PROJECT","QUIZ","LAB","EXAM"};

    int score = 0;
    bool quit = false;
    SDL_Event e;
    Uint32 lastSpawnTime = SDL_GetTicks();

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                bullets.push_back(Bullet{ SDL_Rect{player.x + player.w/2 - 5, player.y, 10, 20} });
                Mix_PlayChannel(-1, shootSnd, 0);
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_LEFT]  && player.x > 0) player.x -= 7;
        if (keys[SDL_SCANCODE_RIGHT] && player.x < SCREEN_WIDTH - player.w) player.x += 7;

        for (auto& b : bullets) b.rect.y += b.speed;
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet& b){ return b.rect.y < 0; }), bullets.end());

        Uint32 current = SDL_GetTicks();
        if (current - lastSpawnTime > 1000) {
            Enemy en; en.rect = { rand() % (SCREEN_WIDTH-60), 0, 60, 40 };
            en.label = labels[rand()%4]; en.speed = 2 + rand()%3;
            enemies.push_back(en);
            lastSpawnTime = current;
        }

        for (auto& en : enemies) en.rect.y += en.speed;

        for (size_t i = 0; i < bullets.size(); ++i) {
            for (size_t j = 0; j < enemies.size(); ++j) {
                if (checkCollision(bullets[i].rect, enemies[j].rect)) {
                    bullets.erase(bullets.begin()+i);
                    enemies.erase(enemies.begin()+j);
                    score += 10;
                    goto POST_COLLISION;
                }
            }
        }
        POST_COLLISION:;

        for (auto& en : enemies) {
            if (en.rect.y > SCREEN_HEIGHT) {
                showEndScreen(renderer, font, score, false);
                Mix_HaltMusic(); Mix_FreeMusic(bgm); Mix_FreeChunk(shootSnd);
                SDL_DestroyTexture(bgTex); SDL_DestroyTexture(playerTex); SDL_DestroyTexture(enemyTex);
                TTF_CloseFont(font);
                return false;
            }
        }

        if (score >= WIN_SCORE) quit = true;

        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTex, NULL, NULL);
        SDL_RenderCopy(renderer, playerTex, NULL, &player);

        for (auto& en : enemies) {
            SDL_RenderCopy(renderer, enemyTex, NULL, &en.rect);
            SDL_Color glow = {(Uint8)(128 + 127 * sin(SDL_GetTicks()/300.0)), 200, 255, 255};
            renderText(renderer, font, en.label, glow, en.rect.x+5, en.rect.y+10);
        }

        SDL_SetRenderDrawColor(renderer, 255,255,0,255);
        for (auto& b : bullets) SDL_RenderFillRect(renderer, &b.rect);

        renderText(renderer, font, "Score: " + std::to_string(score), {255,255,255,255}, 10, 10);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    bool won = score >= WIN_SCORE;
    showEndScreen(renderer, font, score, won);

    Mix_HaltMusic(); Mix_FreeMusic(bgm); Mix_FreeChunk(shootSnd);
    SDL_DestroyTexture(bgTex); SDL_DestroyTexture(playerTex); SDL_DestroyTexture(enemyTex);
    TTF_CloseFont(font);

    return won;
}
