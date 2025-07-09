#include "monster_game.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "../../common/GameContext.h"
#include "../../GameManager.h"
#include "../../UI/leaderboard.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <chrono>

const int SCREEN_W = 800;
const int SCREEN_H = 600;
const int GAME_OVER_DISPLAY_TIME = 2000;

struct Vec2
{
    float x, y;
};
struct Entity
{
    Vec2 pos;
    int health;
};
struct Bullet
{
    Vec2 pos, vel;
    bool active;
};

static Entity player = {{121, 0}, 100};
static Entity monster = {{569, 0}, 100};
static std::vector<Bullet> playerBullets, monsterBullets;
static float monsterTimer = 0, monsterInterval = 2.0f;
static float monsterMoveTimer = 0, monsterMoveInterval = 1.0f;
static bool gameOver = false;
static bool playerWon = false;
static Uint32 gameOverStartTime = 0;
static bool paused = false;

float clamp(float v, float lo, float hi)
{
    return std::max(lo, std::min(v, hi));
}

void DrawBar(SDL_Renderer *R, Vec2 p, int health, SDL_Color col)
{
    SDL_Rect bg = {int(p.x), int(p.y), 100, 10};
    SDL_SetRenderDrawColor(R, 100, 100, 100, 255);
    SDL_RenderFillRect(R, &bg);
    SDL_Rect fg = {int(p.x), int(p.y), health, 10};
    SDL_SetRenderDrawColor(R, col.r, col.g, col.b, col.a);
    SDL_RenderFillRect(R, &fg);
}

void Shoot(std::vector<Bullet> &B, Vec2 pos, Vec2 vel)
{
    B.push_back({pos, vel, true});
}

void UpdateBullets(std::vector<Bullet> &B, float dt)
{
    for (auto &b : B)
        if (b.active)
        {
            b.pos.x += b.vel.x * dt;
            b.pos.y += b.vel.y * dt;
            if (b.pos.x < 0 || b.pos.x > SCREEN_W || b.pos.y < 0 || b.pos.y > SCREEN_H)
                b.active = false;
        }
}

bool CircleRect(Vec2 c, float r, SDL_Rect R)
{
    float nearestX = clamp(c.x, float(R.x), float(R.x + R.w));
    float nearestY = clamp(c.y, float(R.y), float(R.y + R.h));
    float dx = c.x - nearestX;
    float dy = c.y - nearestY;
    return (dx * dx + dy * dy) < r * r;
}

void runMonsterGame(SDL_Renderer *ren, GameContext &ctx)
{
    SDL_Texture *texBG = IMG_LoadTexture(ren, "assets/images/monster_background.png");
    SDL_Texture *texHero = IMG_LoadTexture(ren, "assets/images/hero.png");
    SDL_Texture *texEnem = IMG_LoadTexture(ren, "assets/images/enemy.png");
    SDL_Texture *texPB = IMG_LoadTexture(ren, "assets/images/bullet_player.png");
    SDL_Texture *texEB = IMG_LoadTexture(ren, "assets/images/bullet_enemy.png");

    Mix_Music *bgm = Mix_LoadMUS("assets/audio/starwars.wav");
    Mix_Chunk *sfxShootP = Mix_LoadWAV("assets/audio/shoot_player.mp3");
    Mix_Chunk *sfxShootE = Mix_LoadWAV("assets/audio/shoot_enemy.mp3");

    TTF_Font *font = TTF_OpenFont("assets/fonts/CALIBRIL.TTF", 48);
    if (!texBG || !texHero || !texEnem || !texPB || !texEB || !bgm || !sfxShootP || !sfxShootE || !font)
    {
        SDL_Log("Asset load error: %s", SDL_GetError());
        return;
    }

    Mix_PlayMusic(bgm, -1);

    float heroScale = (100.0f / 64.0f) * 2.5f;
    float enemScale = (100.0f / 64.0f) * 2.5f;

    player = {{121, 400}, 100};
    monster = {{569, SCREEN_H - 100 - 64 * enemScale}, 100};
    playerBullets.clear();
    monsterBullets.clear();
    monsterTimer = monsterMoveTimer = 0;
    gameOver = paused = false;

    bool running = true;
    Uint32 last = SDL_GetTicks();

    while (running)
    {
        Uint32 now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                return;
            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    paused = !paused;
                if (!paused && !gameOver && e.key.keysym.sym == SDLK_SPACE)
                {
                    Mix_PlayChannel(-1, sfxShootP, 0);
                    Vec2 bulletStart = {player.pos.x + (64 * heroScale) / 2 - 8, player.pos.y + (64 * heroScale) / 2 - 8};
                    Shoot(playerBullets, bulletStart, {300, 0});
                }
            }
        }

        if (!paused && !gameOver)
        {
            const Uint8 *ks = SDL_GetKeyboardState(NULL);
            float speed = 200.0f, dx = 0, dy = 0;
            if (ks[SDL_SCANCODE_LEFT])
                dx -= 1;
            if (ks[SDL_SCANCODE_RIGHT])
                dx += 1;
            if (ks[SDL_SCANCODE_UP])
                dy -= 1;
            if (ks[SDL_SCANCODE_DOWN])
                dy += 1;
            float len = std::sqrt(dx * dx + dy * dy);
            if (len)
            {
                dx /= len;
                dy /= len;
                player.pos.x += dx * speed * dt;
                player.pos.y += dy * speed * dt;
            }
            player.pos.x = clamp(player.pos.x, 0, SCREEN_W - 64 * heroScale);
            player.pos.y = clamp(player.pos.y, 0, SCREEN_H - 64 * heroScale);

            monsterTimer += dt;
            if (monsterTimer >= monsterInterval)
            {
                Mix_PlayChannel(-1, sfxShootE, 0);
                for (int i = 0; i < 3; ++i)
                {
                    float offsetY = float(rand() % int(64 * enemScale));
                    float dxm = player.pos.x - monster.pos.x;
                    float dym = player.pos.y - monster.pos.y;
                    float baseAngle = atan2f(dym, dxm);
                    float deviation = ((rand() % 2001) - 1000) / 100.0f;
                    float finalAngle = baseAngle + deviation * M_PI / 180.0f;
                    Vec2 velocity = {cosf(finalAngle) * 300, sinf(finalAngle) * 300};
                    Vec2 bulletStart = {monster.pos.x + 64 * enemScale / 2 - 8, monster.pos.y + offsetY};
                    Shoot(monsterBullets, bulletStart, velocity);
                }
                monsterTimer = 0;
            }

            monsterMoveTimer += dt;
            if (monsterMoveTimer >= monsterMoveInterval)
            {
                monster.pos.x = rand() % (SCREEN_W - int(64 * enemScale));
                monster.pos.y = rand() % (SCREEN_H - int(64 * enemScale));
                monsterMoveTimer = 0;
            }

            UpdateBullets(playerBullets, dt);
            UpdateBullets(monsterBullets, dt);

            SDL_Rect mR = {int(monster.pos.x), int(monster.pos.y), int(64 * enemScale), int(64 * enemScale)};
            SDL_Rect pR = {int(player.pos.x), int(player.pos.y), int(64 * heroScale), int(64 * heroScale)};
            for (auto &b : playerBullets)
                if (b.active && CircleRect(b.pos, 5, mR))
                {
                    monster.health -= 1;
                    b.active = false;
                }
            for (auto &b : monsterBullets)
                if (b.active && CircleRect(b.pos, 5, pR))
                {
                    player.health -= 3;
                    b.active = false;
                }

            if (player.health <= 0 || monster.health <= 0)
            {
                gameOver = true;
                playerWon = (monster.health <= 0);
                gameOverStartTime = SDL_GetTicks();
            }
        }

        if (gameOver && SDL_GetTicks() - gameOverStartTime >= GAME_OVER_DISPLAY_TIME)
        {
            // Stop the sound when game is over (win/lose)
            Mix_HaltMusic();
            Mix_HaltChannel(-1);

            running = false;
        }

        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, texBG, nullptr, nullptr);
        SDL_Rect dstH = {int(player.pos.x), int(player.pos.y), int(64 * heroScale), int(64 * heroScale)};
        SDL_Rect dstM = {int(monster.pos.x), int(monster.pos.y), int(64 * enemScale), int(64 * enemScale)};
        SDL_RenderCopy(ren, texHero, nullptr, &dstH);
        SDL_RenderCopy(ren, texEnem, nullptr, &dstM);
        DrawBar(ren, {player.pos.x - 30, player.pos.y - 20}, player.health, {0, 255, 0, 255});
        DrawBar(ren, {monster.pos.x - 30, monster.pos.y - 20}, monster.health, {255, 165, 0, 255});

        for (auto &b : playerBullets)
            if (b.active)
            {
                SDL_Rect d = {int(b.pos.x), int(b.pos.y), 16, 16};
                SDL_RenderCopy(ren, texPB, nullptr, &d);
            }
        for (auto &b : monsterBullets)
            if (b.active)
            {
                SDL_Rect d = {int(b.pos.x), int(b.pos.y), 16, 16};
                SDL_RenderCopy(ren, texEB, nullptr, &d);
            }

        if (paused)
        {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface *surf = TTF_RenderText_Solid(font, "PAUSED", white);
            SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
            SDL_Rect rect = {SCREEN_W / 2 - surf->w / 2, SCREEN_H / 2 - surf->h / 2, surf->w, surf->h};
            SDL_RenderCopy(ren, tex, nullptr, &rect);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        if (gameOver)
        {
            SDL_Color red = {255, 0, 0, 255};
            const char *msg = playerWon ? "YOU WIN!" : "YOU LOSE!";
            SDL_Surface *surf = TTF_RenderText_Solid(font, msg, red);
            SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
            SDL_Rect dst = {SCREEN_W / 2 - surf->w / 2, SCREEN_H / 2 - surf->h / 2, surf->w, surf->h};
            SDL_RenderCopy(ren, tex, nullptr, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        SDL_RenderPresent(ren);
    }

    if (playerWon)
    {
        TTF_Font *font = TTF_OpenFont("assets/fonts/arial.ttf", 36);
        if (font)
        {
            Leaderboard leaderboard(font);
            leaderboard.loadFromFile("leaderboard.txt");

            using namespace std::chrono;
            float timeSpent = duration_cast<duration<float>>(steady_clock::now() - ctx.startTime).count();

            // Divide by 100 to convert to seconds
            timeSpent /= 100.0f;

            // Add only if leaderboard has < 5 players or current time is better than the slowest
            if (leaderboard.players.size() < 5 || timeSpent < leaderboard.players.back().time)
            {
                leaderboard.players.emplace_back(ctx.playerName, timeSpent);

                // Sort in descending order based on timeSpent (fastest times at the top)
                std::sort(leaderboard.players.begin(), leaderboard.players.end(), [](const Player &a, const Player &b)
                          {
                              return a.time < b.time; // Ascending order for fastest times at the top
                          });

                if (leaderboard.players.size() > 5)
                    leaderboard.players.resize(5); // Keep only the top 5 players

                leaderboard.saveToFile("leaderboard.txt");
            }

            TTF_CloseFont(font);
        }

        ctx.nextState = MENU; // Go back to menu after leaderboard update
    }
    else
    {
        ctx.nextState = MENU;
    }
}