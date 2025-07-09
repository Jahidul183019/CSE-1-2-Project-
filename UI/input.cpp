#include "input.h"
#include "GameContext.h" 
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>

bool getPlayerName(std::string& playerName, TTF_Font* font, SDL_Renderer* renderer, SDL_Window* window) {
    if (!renderer || !window) {
        std::cerr << "Invalid renderer or window context!" << std::endl;
        return false;
    }

    SDL_Texture* background = nullptr;
    SDL_Surface* backgroundSurface = IMG_Load("assets/images/back.png");
    if (!backgroundSurface) {
        std::cerr << "IMG_Load Error: " << IMG_GetError() << std::endl;
        return false;
    }
    background = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);

    if (!background) {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!font) {
        std::cerr << "Font not initialized!\n";
        SDL_DestroyTexture(background);
        return false;
    }

    SDL_Color textColor = {255, 255, 255, 255};
    bool quit = false;
    SDL_Event e;
    std::string promptText = "Enter your name:";
    std::string nameInput = "";
    const int MAX_NAME_LENGTH = 15;
    const int BOX_WIDTH = 400;
    SDL_Rect nameInputBox = {160, 120, 400, 50};

    SDL_StartTextInput();

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                playerName = "";
                SDL_StopTextInput();
                SDL_DestroyTexture(background);
                return false;
            } else if (e.type == SDL_TEXTINPUT) {
                if (nameInput.length() < MAX_NAME_LENGTH) {
                    nameInput += e.text.text;
                }
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKSPACE && !nameInput.empty()) {
                nameInput.pop_back();
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                playerName = nameInput;
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, NULL, NULL);

        // Prompt text
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, promptText.c_str(), textColor);
        SDL_Texture* promptTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect promptRect = {nameInputBox.x, nameInputBox.y - 60, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, promptTexture, NULL, &promptRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(promptTexture);

        // Input box
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &nameInputBox);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &nameInputBox);

        // Characters
        int charWidth = BOX_WIDTH / MAX_NAME_LENGTH;
        for (size_t i = 0; i < nameInput.length(); ++i) {
            std::string ch(1, nameInput[i]);
            SDL_Surface* charSurface = TTF_RenderText_Blended(font, ch.c_str(), textColor);
            SDL_Texture* charTexture = SDL_CreateTextureFromSurface(renderer, charSurface);
            SDL_Rect charRect = {
                nameInputBox.x + static_cast<int>(i) * charWidth,
                nameInputBox.y + (nameInputBox.h - charSurface->h) / 2,
                charWidth,
                charSurface->h
            };
            SDL_RenderCopy(renderer, charTexture, NULL, &charRect);
            SDL_FreeSurface(charSurface);
            SDL_DestroyTexture(charTexture);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    SDL_DestroyTexture(background);
    return true;
}

// ✅ Updated: return bool here too
bool getPlayerName(GameContext& ctx, TTF_Font* font) {
    return getPlayerName(ctx.playerName, font, ctx.renderer, ctx.window);
}
