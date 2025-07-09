#include "leaderboard.h"
#include <fstream>
#include <sstream>
#include <SDL2/SDL_image.h>

// Helper to load image into texture
SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "IMG_Load Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void Leaderboard::loadFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    std::string line;
    players.clear();

    while (std::getline(inFile, line)) {
        size_t commaPos = line.find(',');
        if (commaPos != std::string::npos) {
            std::string name = line.substr(0, commaPos);
            try {
                float time = std::stof(line.substr(commaPos + 1));
                players.emplace_back(name, time);
            } catch (...) {
                std::cerr << "Invalid time entry for: " << name << "\n";
            }
        }
    }
}

void Leaderboard::saveToFile(const std::string& filename) {
    std::ofstream outFile(filename);
    for (const auto& player : players) {
        outFile << player.name << "," << player.time << "\n";
    }
}

void Leaderboard::renderText(const std::string& text, int x, int y, SDL_Color color, SDL_Renderer* renderer) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

bool Leaderboard::handleButtonClick(Button& button, SDL_Renderer* renderer, bool& backToMenu) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_Color btnColor = button.color;

    bool hovered = mouseX > button.rect.x && mouseX < button.rect.x + button.rect.w &&
                   mouseY > button.rect.y && mouseY < button.rect.y + button.rect.h;

    if (hovered) {
        btnColor = {0, 255, 255, 255};  // Cyan hover
        button.rect.w = 250;
        button.rect.h = 55;
    } else {
        button.rect.w = 220;
        button.rect.h = 50;
    }

    renderText(button.label, button.rect.x + (button.rect.w - 100) / 2, button.rect.y + (button.rect.h - 30) / 2, btnColor, renderer);

    if (hovered && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        if (button.label == "BACK TO MENU") backToMenu = true;
        return true;
    }

    return false;
}

void Leaderboard::renderLeaderboard(SDL_Renderer* renderer) {
    SDL_Color textColor = {255, 255, 255, 255};
    int y = 100;

    SDL_Texture* background = loadTexture("assets/images/back.png", renderer);
    SDL_RenderClear(renderer);
    if (background) SDL_RenderCopy(renderer, background, nullptr, nullptr);

    for (const auto& player : players) {
        std::string line = player.name + " - " + std::to_string(player.time) + " seconds";
        renderText(line, 100, y, textColor, renderer);
        y += 50;
    }

    Button backBtn(250, 600, 220, 50, "BACK TO MENU", {255, 0, 0, 255});
    bool temp = false;
    handleButtonClick(backBtn, renderer, temp);

    SDL_RenderPresent(renderer);
    if (background) SDL_DestroyTexture(background);
}

void Leaderboard::openLeaderboardWindow() {
    SDL_Window* window = SDL_CreateWindow("Leaderboard", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 720, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    bool backToMenu = false;

    while (!backToMenu) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                backToMenu = true;
                break;
            }
        }

        Button backBtn(250, 600, 220, 50, "BACK TO MENU", {255, 0, 0, 255});
        if (handleButtonClick(backBtn, renderer, backToMenu)) {
            break;
        }

        renderLeaderboard(renderer);
        SDL_Delay(16);  // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
