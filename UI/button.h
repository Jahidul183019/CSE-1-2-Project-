#ifndef BUTTON_H
#define BUTTON_H

#include <SDL2/SDL.h>
#include <string>

struct Button {
    SDL_Rect rect;
    std::string label;
    SDL_Color color;
    bool isHovered;

    Button(int x, int y, int w, int h, const std::string& text, SDL_Color col)
        : rect{x, y, w, h}, label(text), color(col), isHovered(false) {}
};

#endif // BUTTON_H