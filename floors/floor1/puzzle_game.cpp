#include "puzzle_game.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <vector>
#include <iostream>
#include "../../common/utils.h"
#include "rsa_game.h"
#include "../../common/game_state.h"

using namespace std;

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;
const int PUZZLE_TIME_LIMIT = 30;

struct Puzzle {
    string question;
    string answer;
};
bool runPuzzleGame(SDL_Renderer* renderer) {
    TTF_Font* font = TTF_OpenFont("assets/fonts/impact.ttf", 24);
    if (!font) return false;

    SDL_Texture* bgTexture = loadTexture(renderer, "assets/images/puzzleimage.png");
    SDL_Texture* decryptTex = loadTexture(renderer, "assets/images/decryptor.png");
    Mix_Music* bgm = Mix_LoadMUS("assets/audio/puzzleGame.wav");
    Mix_Chunk* correctSfx = Mix_LoadWAV("assets/audio/correct.mp3");
    Mix_Chunk* wrongSfx = Mix_LoadWAV("assets/audio/wrong.mp3");

    if (bgm) Mix_PlayMusic(bgm, -1);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Event e;

    vector<Puzzle> puzzles = {
        {"I have keys but no locks, I have space but no room. What am I?", "keyboard"},
        {"What has to be broken before you use it?", "egg"},
        {"Crimson frames hold knowledge tight,\nWhere daylight meets the scholar's light.", "curzon"}
    };

    int currentPuzzle = -1;
    bool running = true, puzzleStarted = false, puzzleSolved = false;
    Uint32 puzzleStartTime = 0;
    string userInput;
    SDL_Rect monitorTouchArea = {320, 256, 512, 320};

    while (running) {
        if (puzzleStarted && !SDL_IsTextInputActive()) SDL_StartTextInput();
        if ((!puzzleStarted || puzzleSolved) && SDL_IsTextInputActive()) SDL_StopTextInput();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                // Full cleanup
                Mix_HaltMusic();
                Mix_FreeMusic(bgm);
                Mix_FreeChunk(correctSfx);
                Mix_FreeChunk(wrongSfx);
                SDL_DestroyTexture(bgTexture);
                SDL_DestroyTexture(decryptTex);
                TTF_CloseFont(font);
                SDL_StopTextInput();
                return false;
            }

            if (!puzzleStarted && e.type == SDL_MOUSEBUTTONDOWN) {
                int mx, my; SDL_GetMouseState(&mx, &my);
                if (mx >= monitorTouchArea.x && mx <= monitorTouchArea.x + monitorTouchArea.w &&
                    my >= monitorTouchArea.y && my <= monitorTouchArea.y + monitorTouchArea.h) {
                    currentPuzzle = 0;
                    puzzleStarted = true;
                    puzzleSolved = false;
                    userInput.clear();
                    puzzleStartTime = SDL_GetTicks();
                }
            }

            if (puzzleStarted && !puzzleSolved) {
                if (e.type == SDL_TEXTINPUT) userInput += e.text.text;
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_BACKSPACE && !userInput.empty()) userInput.pop_back();
                    else if (e.key.keysym.sym == SDLK_RETURN) {
                        if (userInput == puzzles[currentPuzzle].answer) {
                            Mix_PlayChannel(-1, correctSfx, 0);
                            puzzleSolved = true;  // Only set this to true if the answer is correct
                        }
                    }
                }
            }

            if (puzzleSolved && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                currentPuzzle++;
                if (currentPuzzle < (int)puzzles.size()) {
                    puzzleStarted = true;
                    puzzleSolved = false;
                    userInput.clear();
                    puzzleStartTime = SDL_GetTicks();
                } else {
                    SDL_StopTextInput();
                    running = false;
                }
            }
        }

        if (puzzleStarted && !puzzleSolved) {
            Uint32 now = SDL_GetTicks();
            int secondsLeft = PUZZLE_TIME_LIMIT - (now - puzzleStartTime) / 1000;
            if (secondsLeft <= 0) {
                Mix_PlayChannel(-1, wrongSfx, 0);
                SDL_Delay(1500);
                SDL_StopTextInput();
                SDL_DestroyTexture(bgTexture);
                SDL_DestroyTexture(decryptTex);
                TTF_CloseFont(font);
                Mix_HaltMusic();
                Mix_FreeMusic(bgm);
                Mix_FreeChunk(correctSfx);
                Mix_FreeChunk(wrongSfx);
                return false;  // Return false if time runs out
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        if (bgTexture) SDL_RenderCopy(renderer, bgTexture, nullptr, nullptr);

        SDL_Rect r; SDL_Texture* txt = nullptr;
        if (!puzzleStarted) {
            txt = renderText(renderer, font, "Click screen to start puzzle. Use lowercase answers only.", white, r);
            r.x = (SCREEN_WIDTH - r.w) / 2;
            r.y = SCREEN_HEIGHT - 300;
        } else if (puzzleSolved) {
            txt = renderText(renderer, font, "Correct! Press SPACE for next puzzle.", white, r);
            r.x = (SCREEN_WIDTH - r.w) / 2;
            r.y = 100;
        } else {
            Uint32 now = SDL_GetTicks();
            int secondsLeft = PUZZLE_TIME_LIMIT - (now - puzzleStartTime) / 1000;
            string full = puzzles[currentPuzzle].question + "\nYour Answer: " + userInput +
                          "\nTime Left: " + to_string(secondsLeft);
            txt = renderText(renderer, font, full, white, r);
            r.x = (SCREEN_WIDTH - r.w) / 2;
            r.y = 100;
        }

        if (txt) {
            SDL_RenderCopy(renderer, txt, nullptr, &r);
            SDL_DestroyTexture(txt);
        }

        SDL_RenderPresent(renderer);
    }

    // Show decryptor image for 2 seconds
    if (decryptTex) {
        Uint32 start = SDL_GetTicks();
        while (SDL_GetTicks() - start < 2000) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_QUIT) {
                    Mix_HaltMusic();
                    Mix_FreeMusic(bgm);
                    Mix_FreeChunk(correctSfx);
                    Mix_FreeChunk(wrongSfx);
                    SDL_DestroyTexture(bgTexture);
                    SDL_DestroyTexture(decryptTex);
                    TTF_CloseFont(font);
                    return false;
                }
            }
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, decryptTex, nullptr, nullptr);
            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        }
    }

    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(decryptTex);
    TTF_CloseFont(font);
    Mix_HaltMusic();
    Mix_FreeMusic(bgm);
    Mix_FreeChunk(correctSfx);
    Mix_FreeChunk(wrongSfx);

    return true;  // Return true if the puzzle is successfully solved
}