#ifndef UTILS_H
#define UTILS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

// Load image file and return texture
SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path);

// Render text and return texture (fills rectOut with size)
SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font,
                        const std::string& text, SDL_Color color,
                        SDL_Rect& rectOut, int wrapLength = 800);

#endif // UTILS_H
