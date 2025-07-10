#include "menu.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "input.h"

const SDL_Color BUTTON_COLOR = {70, 130, 180, 255};
const SDL_Color BUTTON_HOVER = {100, 180, 255, 255};
const SDL_Color TEXT_COLOR = {255, 255, 255, 255};

const std::vector<std::string> BUTTON_LABELS = {
    "STORY", "NEW GAME", "MAP", "LEADERBOARD", "CREDITS", "EXIT"
};

std::vector<std::string> wrapText(const std::string& text, TTF_Font* font, int maxWidth) {
    std::vector<std::string> lines;
    std::string currentLine;
    std::istringstream words(text);
    std::string word;

    while (words >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        SDL_Surface* testSurface = TTF_RenderText_Blended(font, testLine.c_str(), TEXT_COLOR);
        if (testSurface && testSurface->w > maxWidth) {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine = word;
            }
        } else {
            currentLine = testLine;
        }
        SDL_FreeSurface(testSurface);
    }

    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    return lines;
}

GameState runMenu(GameContext& ctx) {
    SDL_Renderer* renderer = ctx.renderer;
    SDL_Window* window = ctx.window;

    SDL_Texture* bg = IMG_LoadTexture(renderer, "assets/images/menu.png");
    if (!bg) std::cerr << "Failed to load menu.png: " << IMG_GetError() << std::endl;

    Mix_Music* menuMusic = Mix_LoadMUS("assets/audio/menu_background.mp3");
    if (menuMusic) {
        Mix_PlayMusic(menuMusic, -1);
    } else {
        std::cerr << "Failed to load menu music: " << Mix_GetError() << std::endl;
    }

    TTF_Font* font = TTF_OpenFont("assets/fonts/OpenSans-Bold.ttf", 36);
    TTF_Font* titleFont = TTF_OpenFont("assets/fonts/OpenSans-Bold.ttf", 48);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        if (bg) SDL_DestroyTexture(bg);
        if (Mix_PlayingMusic()) Mix_HaltMusic();
        if (menuMusic) Mix_FreeMusic(menuMusic);
        return EXIT;
    }

    std::vector<Button> buttons;
    int btnWidth = 300, btnHeight = 60;
    int startY = 160, spacing = 10;
    for (size_t i = 0; i < BUTTON_LABELS.size(); ++i) {
        int x = (720 - btnWidth) / 2;
        int y = startY + i * (btnHeight + spacing);
        buttons.emplace_back(x, y, btnWidth, btnHeight, BUTTON_LABELS[i], BUTTON_COLOR);
    }

    bool running = true;
    bool showingMap = false, showingLeaderboard = false, showingStory = false, showingCredits = false;
    GameState result = EXIT;
    SDL_Event e;

    SDL_Texture* mapTex = nullptr, *leaderboardBgTex = nullptr, *storyBgTex = nullptr, *clickedImage = nullptr;
    Button backButton((720 - 200) / 2, 500, 200, 50, "MENU", BUTTON_COLOR);

    std::vector<std::string> leaderboardLines;
    std::string storyText;

    std::ifstream storyFile("story.txt");
    if (!storyFile) {
        std::cerr << "Failed to open story.txt" << std::endl;
        return EXIT;
    }
    std::string line;
    while (std::getline(storyFile, line)) storyText += line + "\n";

    int scrollOffset = 0, totalHeight = 0;
    const int scrollSpeed = 20, maxHeight = 600, maxWidth = 700;
    std::vector<std::string> wrappedText = wrapText(storyText, font, maxWidth);
    for (const auto& line : wrappedText) {
        SDL_Surface* surface = TTF_RenderText_Blended(font, line.c_str(), TEXT_COLOR);
        totalHeight += surface->h + 10;
        SDL_FreeSurface(surface);
    }
    int maxScrollOffset = std::max(0, totalHeight - maxHeight);

    std::vector<std::string> names = {"Jahid", "Apon", "Soumik", "Turja"};
    std::vector<SDL_Rect> nameRects;
    std::string clickedName = "";

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                result = EXIT;
            } else if (e.type == SDL_MOUSEMOTION) {
                int mx = e.motion.x, my = e.motion.y;
                SDL_Point pt = {mx, my};
                backButton.isHovered = (showingStory || showingLeaderboard || showingMap || showingCredits) && SDL_PointInRect(&pt, &backButton.rect);
                if (!showingMap && !showingLeaderboard && !showingStory && !showingCredits) {
                    for (auto& btn : buttons)
                        btn.isHovered = SDL_PointInRect(&pt, &btn.rect);
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x, my = e.button.y;
                SDL_Point pt = {mx, my};
                if ((showingMap || showingLeaderboard || showingStory || showingCredits) && SDL_PointInRect(&pt, &backButton.rect)) {
                    showingMap = showingLeaderboard = showingStory = showingCredits = false;
                    if (mapTex) { SDL_DestroyTexture(mapTex); mapTex = nullptr; }
                    if (leaderboardBgTex) { SDL_DestroyTexture(leaderboardBgTex); leaderboardBgTex = nullptr; }
                    if (storyBgTex) { SDL_DestroyTexture(storyBgTex); storyBgTex = nullptr; }
                    if (clickedImage) { SDL_DestroyTexture(clickedImage); clickedImage = nullptr; }
                    clickedName = "";
                } else if (!showingMap && !showingLeaderboard && !showingStory && !showingCredits) {
                    for (const auto& btn : buttons) {
                        if (SDL_PointInRect(&pt, &btn.rect)) {
                            if (btn.label == "NEW GAME") {
                                resetGameProgress();
                                if (getPlayerName(ctx, font)) result = FLOOR1;
                                else result = MENU;
                                running = false;
                                break;
                            } else if (btn.label == "MAP") {
                                mapTex = IMG_LoadTexture(renderer, "assets/images/map.png");
                                if (!mapTex) std::cerr << "Failed to load map.png: " << IMG_GetError() << std::endl;
                                else showingMap = true;
                            } else if (btn.label == "LEADERBOARD") {
                                leaderboardBgTex = IMG_LoadTexture(renderer, "assets/images/leader.png");
                                if (!leaderboardBgTex) std::cerr << "Failed to load leaderboard background: " << IMG_GetError() << std::endl;
                                else {
                                    showingLeaderboard = true;
                                    std::ifstream file("leaderboard.txt");
                                    leaderboardLines.clear();
                                    std::string line;
                                    while (std::getline(file, line)) leaderboardLines.push_back(line);
                                }
                            } else if (btn.label == "STORY") {
                                storyBgTex = IMG_LoadTexture(renderer, "assets/images/back.png");
                                showingStory = true;
                            } else if (btn.label == "CREDITS") {
                                showingCredits = true;
                            } else if (btn.label == "EXIT") {
                                result = EXIT;
                                running = false;
                                break;
                            }
                        }
                    }
                } else if (showingCredits) {
                    for (size_t i = 0; i < nameRects.size(); ++i) {
                        if (SDL_PointInRect(&pt, &nameRects[i])) {
                            clickedName = names[i];
                            if (clickedImage) {
                                SDL_DestroyTexture(clickedImage);
                                clickedImage = nullptr;
                            }
                            std::string path = "assets/images/credits/" + clickedName + ".png";
                            clickedImage = IMG_LoadTexture(renderer, path.c_str());
                            if (!clickedImage) std::cerr << "Failed to load image: " << IMG_GetError() << std::endl;
                        }
                    }
                }
            } else if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.y > 0) scrollOffset = std::max(0, scrollOffset - scrollSpeed);
                else scrollOffset = std::min(maxScrollOffset, scrollOffset + scrollSpeed);
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        

        if (showingCredits) {
            SDL_Texture* creditsBg = IMG_LoadTexture(renderer, "assets/images/back.png");
if (creditsBg) {
    SDL_RenderCopy(renderer, creditsBg, nullptr, nullptr);
}
SDL_Rect imgRect = {0, 0, 720, 720};
SDL_RenderCopy(renderer, clickedImage, nullptr, &imgRect);
if (creditsBg) {
    SDL_DestroyTexture(creditsBg);
}

            int y = 150;
            nameRects.clear();
            for (const std::string& name : names) {
                SDL_Surface* surf = TTF_RenderText_Blended(font, name.c_str(), TEXT_COLOR);
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_Rect rect = {(720 - surf->w) / 2, y, surf->w, surf->h};
                nameRects.push_back(rect);
                SDL_RenderCopy(renderer, tex, nullptr, &rect);
                y += surf->h + 20;
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(tex);
                
            }
            
            if (clickedImage) {
                SDL_Rect imgRect = {0, 0, 720, 720};
                SDL_RenderCopy(renderer, clickedImage, nullptr, &imgRect);

            }
        }else {
            if (bg) SDL_RenderCopy(renderer, bg, nullptr, nullptr);
            if (titleFont) {
                SDL_Surface* titleSurf = TTF_RenderText_Blended(titleFont, "ESCAPE ROOM CONQUEST", TEXT_COLOR);
                SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
                SDL_Rect titleRect = {(720 - titleSurf->w) / 2 + 50, 80, titleSurf->w, titleSurf->h};
                SDL_RenderCopy(renderer, titleTex, nullptr, &titleRect);
                SDL_FreeSurface(titleSurf);
                SDL_DestroyTexture(titleTex);
            }
            for (const auto& btn : buttons) {
                SDL_SetRenderDrawColor(renderer, btn.isHovered ? BUTTON_HOVER.r : btn.color.r, btn.isHovered ? BUTTON_HOVER.g : btn.color.g, btn.isHovered ? BUTTON_HOVER.b : btn.color.b, 255);
                SDL_RenderFillRect(renderer, &btn.rect);
                SDL_Surface* textSurf = TTF_RenderText_Blended(font, btn.label.c_str(), TEXT_COLOR);
                SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurf);
                int tx = btn.rect.x + (btn.rect.w - textSurf->w) / 2;
                int ty = btn.rect.y + (btn.rect.h - textSurf->h) / 2;
                SDL_Rect textRect = {tx, ty, textSurf->w, textSurf->h};
                SDL_RenderCopy(renderer, textTex, nullptr, &textRect);
                SDL_FreeSurface(textSurf);
                SDL_DestroyTexture(textTex);
            }
        }

        if (showingMap || showingLeaderboard || showingStory || showingCredits) {
            SDL_SetRenderDrawColor(renderer, backButton.isHovered ? BUTTON_HOVER.r : backButton.color.r, backButton.isHovered ? BUTTON_HOVER.g : backButton.color.g, backButton.isHovered ? BUTTON_HOVER.b : backButton.color.b, 255);
            SDL_RenderFillRect(renderer, &backButton.rect);
            SDL_Surface* labelSurf = TTF_RenderText_Blended(font, backButton.label.c_str(), TEXT_COLOR);
            SDL_Texture* labelTex = SDL_CreateTextureFromSurface(renderer, labelSurf);
            SDL_Rect labelRect = {backButton.rect.x + (backButton.rect.w - labelSurf->w) / 2, backButton.rect.y + (backButton.rect.h - labelSurf->h) / 2, labelSurf->w, labelSurf->h};
            SDL_RenderCopy(renderer, labelTex, nullptr, &labelRect);
            SDL_FreeSurface(labelSurf);
            SDL_DestroyTexture(labelTex);
        }

        SDL_RenderPresent(renderer);
    }

    if (bg) SDL_DestroyTexture(bg);
    if (mapTex) SDL_DestroyTexture(mapTex);
    if (leaderboardBgTex) SDL_DestroyTexture(leaderboardBgTex);
    if (storyBgTex) SDL_DestroyTexture(storyBgTex);
    if (clickedImage) SDL_DestroyTexture(clickedImage);
    TTF_CloseFont(font);
    if (titleFont) TTF_CloseFont(titleFont);
    if (Mix_PlayingMusic()) Mix_HaltMusic();
    if (menuMusic) Mix_FreeMusic(menuMusic);

    return result;
}