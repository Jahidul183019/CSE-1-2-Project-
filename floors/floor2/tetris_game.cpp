#include "tetris_game.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>

bool runTetrisGame(SDL_Renderer* renderer) {
    const int GAME_WIDTH = 300;
    const int GAME_HEIGHT = 600;
    const int BLOCK_SIZE = 30;
    const int GRID_WIDTH = GAME_WIDTH / BLOCK_SIZE;
    const int GRID_HEIGHT = GAME_HEIGHT / BLOCK_SIZE;

    int grid[GRID_HEIGHT][GRID_WIDTH] = {0};
    int score = 0;
    const int speed = 500;

    struct Tetromino {
        int shape[4][4];
        int x, y, color;
    };

    const int tetrominoShapes[7][4][4] = {
        {{1,1,1,1},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
        {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}
    };

    auto getColor = [](int c) -> SDL_Color {
        SDL_Color colors[8] = {
            {0,0,0,255},{0,255,255,255},{255,255,0,255},{255,0,0,255},
            {0,255,0,255},{255,165,0,255},{0,0,255,255},{128,0,128,255}
        };
        return colors[(c < 0 || c > 7) ? 0 : c];
    };

    auto generateTetromino = [&]() {
        Tetromino t;
        int idx = rand() % 7;
        t.color = idx + 1;
        t.x = GRID_WIDTH / 2 - 2;
        t.y = 0;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                t.shape[i][j] = tetrominoShapes[idx][i][j];
        return t;
    };

    auto checkCollision = [&](const Tetromino& t) {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                if (t.shape[i][j]) {
                    int x = t.x + j, y = t.y + i;
                    if (x < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT || grid[y][x] != 0)
                        return true;
                }
        return false;
    };

    auto placeTetromino = [&](const Tetromino& t) {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                if (t.shape[i][j])
                    grid[t.y + i][t.x + j] = t.color;
    };

    auto clearLines = [&](Mix_Chunk* lineClearSound) {
        for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
            bool full = true;
            for (int x = 0; x < GRID_WIDTH; ++x)
                if (!grid[y][x]) { full = false; break; }

            if (full) {
                Mix_PlayChannel(-1, lineClearSound, 0);
                score += 100;
                for (int i = y; i > 0; --i)
                    for (int x = 0; x < GRID_WIDTH; ++x)
                        grid[i][x] = grid[i - 1][x];
                for (int x = 0; x < GRID_WIDTH; ++x)
                    grid[0][x] = 0;
                y++;
            }
        }
    };

    auto drawBlock = [&](SDL_Renderer* r, int x, int y, int c, int offsetX, int offsetY) {
        SDL_Color col = getColor(c);
        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
        SDL_Rect rect = {offsetX + x * BLOCK_SIZE, offsetY + y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
        SDL_RenderFillRect(r, &rect);
    };

    auto drawTetromino = [&](SDL_Renderer* r, const Tetromino& t, int offsetX, int offsetY) {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                if (t.shape[i][j])
                    drawBlock(r, t.x + j, t.y + i, t.color, offsetX, offsetY);
    };

    Mix_Music* music = Mix_LoadMUS("assets/audio/tetris_background.mp3");
    Mix_Chunk* moveSound = Mix_LoadWAV("assets/audio/move.mp3");
    Mix_Chunk* rotateSound = Mix_LoadWAV("assets/audio/rotate.mp3");
    Mix_Chunk* lineClearSound = Mix_LoadWAV("assets/audio/line_clear.mp3");
    SDL_Texture* backgroundTex = IMG_LoadTexture(renderer, "assets/images/tetris_background.png");
    TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", 24);

    if (music) Mix_PlayMusic(music, -1);
    srand(time(0));
    Tetromino cur = generateTetromino();
    Uint32 last = SDL_GetTicks();
    bool running = true;

    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
    const int offsetX = (SCREEN_WIDTH - GAME_WIDTH) / 2;
    const int offsetY = 0;

    auto cleanup = [&]() {
        Mix_HaltChannel(-1);
        Mix_HaltMusic();
        if (backgroundTex) SDL_DestroyTexture(backgroundTex);
        if (moveSound) Mix_FreeChunk(moveSound);
        if (rotateSound) Mix_FreeChunk(rotateSound);
        if (lineClearSound) Mix_FreeChunk(lineClearSound);
        if (music) Mix_FreeMusic(music);
        if (font) TTF_CloseFont(font);
    };

    auto returnAndCleanup = [&](bool result) {
        cleanup();
        return result;
    };

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                return returnAndCleanup(false);

            if (e.type == SDL_KEYDOWN) {
                Tetromino tmp = cur;
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        cur.x--; if (checkCollision(cur)) cur.x++;
                        Mix_PlayChannel(-1, moveSound, 0);
                        break;
                    case SDLK_RIGHT:
                        cur.x++; if (checkCollision(cur)) cur.x--;
                        Mix_PlayChannel(-1, moveSound, 0);
                        break;
                    case SDLK_DOWN:
                        cur.y++;
                        if (checkCollision(cur)) {
                            cur.y--;
                            placeTetromino(cur);
                            clearLines(lineClearSound);
                            if (score >= 500) return returnAndCleanup(true);
                            cur = generateTetromino();
                            if (checkCollision(cur)) return returnAndCleanup(false);
                        }
                        Mix_PlayChannel(-1, moveSound, 0);
                        break;
                    case SDLK_UP:
                        Mix_PlayChannel(-1, rotateSound, 0);
                        for (int i = 0; i < 4; ++i)
                            for (int j = 0; j < 4; ++j)
                                tmp.shape[i][j] = cur.shape[3 - j][i];
                        if (!checkCollision(tmp)) cur = tmp;
                        break;
                    case SDLK_ESCAPE:
                        return returnAndCleanup(false);
                }
            }
        }

        if (SDL_GetTicks() - last >= (Uint32)speed) {
            cur.y++;
            if (checkCollision(cur)) {
                cur.y--;
                placeTetromino(cur);
                clearLines(lineClearSound);
                if (score >= 500) return returnAndCleanup(true);
                cur = generateTetromino();
                if (checkCollision(cur)) return returnAndCleanup(false);
            }
            last = SDL_GetTicks();
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (backgroundTex) {
            SDL_Rect dst = {offsetX, offsetY, GAME_WIDTH, GAME_HEIGHT};
            SDL_RenderCopy(renderer, backgroundTex, nullptr, &dst);
        }

        for (int y = 0; y < GRID_HEIGHT; ++y)
            for (int x = 0; x < GRID_WIDTH; ++x)
                if (grid[y][x]) drawBlock(renderer, x, y, grid[y][x], offsetX, offsetY);

        drawTetromino(renderer, cur, offsetX, offsetY);

        SDL_Color white = {255, 255, 255, 255};
        char buf[32]; sprintf(buf, "Score: %d", score);
        SDL_Surface* s = TTF_RenderText_Solid(font, buf, white);
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_Rect r = {offsetX + (GAME_WIDTH - s->w) / 2, 5, s->w, s->h};
        SDL_FreeSurface(s);
        SDL_RenderCopy(renderer, t, nullptr, &r);
        SDL_DestroyTexture(t);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    return returnAndCleanup(score >= 500);
}
