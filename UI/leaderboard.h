#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <iostream>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "button.h"

struct Player {
    std::string name;
    float time;

    Player() : name(""), time(0.0f) {}
    Player(const std::string& playerName, float completionTime)
        : name(playerName), time(completionTime) {}

    // ✅ Add this for sorting
    bool operator<(const Player& other) const {
        return time < other.time;
    }
};


class Leaderboard
{
public:
    std::vector<Player> players;
    TTF_Font *font;

    Leaderboard(TTF_Font *font) : font(font) {}

    void loadFromFile(const std::string &filename);
    void saveToFile(const std::string &filename);
    void renderLeaderboard(SDL_Renderer *renderer);
    bool handleButtonClick(Button &button, SDL_Renderer *renderer, bool &backToMenu);
    void renderText(const std::string &text, int x, int y, SDL_Color color, SDL_Renderer *renderer);
    void openLeaderboardWindow();
};

#endif
