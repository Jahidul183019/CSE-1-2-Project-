// utils.cpp

#include "utils.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

// Load an image file and convert it to an SDL_Texture
SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "IMG_Load failed: " << IMG_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << std::endl;
    }

    SDL_FreeSurface(surface);
    return texture;
}

// Render text to an SDL_Texture, returning the texture and setting the rect size
SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font,
                        const std::string& text, SDL_Color color,
                        SDL_Rect& rectOut, int wrapLength) {
    if (text.empty()) {
        // Prevent crash and set rect to zero size
        rectOut = {0, 0, 0, 0};
        return nullptr;
    }

    SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), color, wrapLength);
    if (!surface) {
        std::cerr << "TTF_RenderText_Blended_Wrapped failed: " << TTF_GetError() << std::endl;
        rectOut = {0, 0, 0, 0};  // Safe fallback
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << std::endl;
    }

    rectOut = {0, 0, surface->w, surface->h};
    SDL_FreeSurface(surface);
    return texture;
}
