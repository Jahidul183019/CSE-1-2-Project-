#include "projection_game.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

// Constants
static constexpr int WIDTH         = 800;
static constexpr int HEIGHT        = 600;
static constexpr int CELL          = 40;
static constexpr int MARGIN        = 60;
static constexpr int COLS          = 13;
static constexpr int ROWS          = 11;
static constexpr int GRID_ORIGIN_X = MARGIN;
static constexpr int GRID_ORIGIN_Y = HEIGHT - MARGIN;
static constexpr Uint32 TIME_LIMIT = 60 * 1000; // ms

// 2D vector
struct Vec2 {
    double x, y;
    Vec2 operator+(const Vec2& b) const { return {x+b.x, y+b.y}; }
    Vec2 operator-(const Vec2& b) const { return {x-b.x, y-b.y}; }
    Vec2 operator*(double k)      const { return {x*k, y*k}; }
    double dot(const Vec2& b)     const { return x*b.x + y*b.y; }
    double len()                  const { return std::sqrt(x*x + y*y); }
};

// Convert logical v→screen coords
static void to_screen(const Vec2& v, int& sx, int& sy) {
    sx = GRID_ORIGIN_X + int(std::round(v.x * CELL));
    sy = GRID_ORIGIN_Y - int(std::round(v.y * CELL));
}

// Draw a thick arrow from start→end
static void drawArrow(SDL_Renderer* R, const Vec2& A, const Vec2& B, SDL_Color c, int thick=2) {
    int x1,y1,x2,y2; to_screen(A,x1,y1); to_screen(B,x2,y2);
    SDL_SetRenderDrawColor(R,c.r,c.g,c.b,c.a);
    for(int dx=-thick/2;dx<=thick/2;dx++)
      for(int dy=-thick/2;dy<=thick/2;dy++)
        SDL_RenderDrawLine(R, x1+dx,y1+dy, x2+dx,y2+dy);
    Vec2 v = B - A; double L = v.len();
    if(L>1e-3){
      Vec2 u = v*(1.0/L);
      Vec2 l = {-u.y, u.x}, r = { u.y,-u.x};
      Vec2 h1 = B - u*0.4 - l*0.25, h2 = B - u*0.4 - r*0.25;
      int hx1,hy1,hx2,hy2,ex,ey;
      to_screen(B, ex,ey);
      to_screen(h1, hx1,hy1);
      to_screen(h2, hx2,hy2);
      SDL_RenderDrawLine(R, ex,ey, hx1,hy1);
      SDL_RenderDrawLine(R, ex,ey, hx2,hy2);
    }
}

// Draw a filled circle at v
static void drawPoint(SDL_Renderer* R, const Vec2& v, SDL_Color c, int rad=6) {
    int sx,sy; to_screen(v,sx,sy);
    SDL_SetRenderDrawColor(R,c.r,c.g,c.b,c.a);
    for(int dx=-rad;dx<=rad;dx++)
      for(int dy=-rad;dy<=rad;dy++)
        if(dx*dx+dy*dy<=rad*rad)
          SDL_RenderDrawPoint(R, sx+dx, sy+dy);
}

// Render text at (x,y)
static void renderText(SDL_Renderer* R, TTF_Font* f,
                       const std::string& txt, SDL_Color c, int x, int y)
{
    SDL_Surface* surf = TTF_RenderUTF8_Blended(f, txt.c_str(), c);
    if(!surf) return;
    SDL_Texture* tx = SDL_CreateTextureFromSurface(R, surf);
    SDL_Rect dst{ x,y, surf->w, surf->h };
    SDL_FreeSurface(surf);
    SDL_RenderCopy(R, tx, nullptr, &dst);
    SDL_DestroyTexture(tx);
}

// Strict parse "int,int"
static bool parseVec2(const std::string& s, Vec2& out) {
    std::string t; t.reserve(s.size());
    for(char c:s) if(!std::isspace(c)) t.push_back(c);
    auto pos = t.find(',');
    if(pos==std::string::npos) return false;
    std::string xs = t.substr(0,pos), ys = t.substr(pos+1);
    try {
        size_t i1,i2;
        int x = std::stoi(xs,&i1), y = std::stoi(ys,&i2);
        if(i1!=xs.size()||i2!=ys.size()) return false;
        if(x<0||x>=COLS||y<0||y>=ROWS) return false;
        out = {double(x),double(y)};
        return true;
    } catch(...) {
        return false;
    }
}

// Compare with tolerance
static bool closeEnough(const Vec2& a, const Vec2& b) {
    return std::abs(a.x-b.x)<1e-2 && std::abs(a.y-b.y)<1e-2;
}

// Projection of y onto u
static Vec2 project(const Vec2& y, const Vec2& u) {
    double k = (y.dot(u))/(u.dot(u));
    return u*k;
}

void runProjectionGame(SDL_Renderer* renderer) {
    TTF_Font* font       = TTF_OpenFont("assets/fonts/OpenSans-Bold.ttf", 20);
    Mix_Music* bgm       = Mix_LoadMUS("assets/audio/projection_background.mp3");
    Mix_Chunk* clickSfx  = Mix_LoadWAV("assets/audio/error.mp3");
    Mix_Chunk* winSfx    = Mix_LoadWAV("assets/audio/victory.mp3");
    SDL_Texture* bgTex   = IMG_LoadTexture(renderer, "assets/images/projection_3d_bg.png");

    if (bgm) Mix_PlayMusic(bgm, -1);

start_game:
    Vec2 u1 = {1, 0}, u2 = {0, 1};
    Vec2 u1_vis = u1 * double(COLS - 1), u2_vis = u2 * double(ROWS - 1);
    Vec2 y = {6, 7};

    bool running = true;
    bool showProj = true;
    bool inputMode = false;
    bool winFlag = false;
    int stage = 0;
    std::string userInput, inputErr;
    Vec2 userY1, userY2;

    Uint32 startTime = SDL_GetTicks();
    SDL_StartTextInput();

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                // Block quitting unless won
                if (!winFlag) {
                    if (clickSfx) Mix_PlayChannel(-1, clickSfx, 0);
                    continue;
                } else {
                    running = false;
                    break;
                }
            }

            if (!inputMode && stage < 3 && e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:  if (y.x > 0) y.x--; break;
                    case SDLK_RIGHT: if (y.x < COLS - 1) y.x++; break;
                    case SDLK_DOWN:  if (y.y > 0) y.y--; break;
                    case SDLK_UP:    if (y.y < ROWS - 1) y.y++; break;
                    case SDLK_SPACE: showProj = !showProj; break;
                    case SDLK_RETURN:
                        inputMode = true; stage = 1;
                        userInput.clear(); inputErr.clear();
                        break;
                }
            } else if (inputMode && e.type == SDL_TEXTINPUT) {
                userInput += e.text.text;
            } else if (inputMode && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !userInput.empty())
                    userInput.pop_back();
                else if (e.key.keysym.sym == SDLK_RETURN) {
                    Vec2 parsed;
                    if (parseVec2(userInput, parsed)) {
                        inputErr.clear();
                        if (stage == 1) {
                            userY1 = parsed;
                            stage = 2;
                            userInput.clear();
                        } else if (stage == 2) {
                            userY2 = parsed;
                            stage = 3;
                            inputMode = false;

                            Vec2 c1 = project(y, u1), c2 = project(y, u2);
                            winFlag = closeEnough(userY1, c1) && closeEnough(userY2, c2);

                            if (winFlag) {
                                if (winSfx) Mix_PlayChannel(-1, winSfx, 0);
                            } else {
                                if (clickSfx) Mix_PlayChannel(-1, clickSfx, 0);
                            }
                        }
                    } else {
                        inputErr = "Invalid format, use x,y";
                        if (clickSfx) Mix_PlayChannel(-1, clickSfx, 0);
                    }
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    inputMode = false; stage = 0;
                    userInput.clear(); inputErr.clear();
                }
            }
        }

        // --- timer ---
        Uint32 now = SDL_GetTicks();
        if (!winFlag && now - startTime >= TIME_LIMIT) {
            if (clickSfx) Mix_PlayChannel(-1, clickSfx, 0);
            SDL_Delay(1500);
            goto start_game;  // Restart game on timeout
        }

        // Projections
        Vec2 p1 = project(y, u1), p2 = project(y, u2);

        // --- render ---
        if (bgTex) {
            SDL_RenderCopy(renderer, bgTex, nullptr, nullptr);
        } else {
            SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
            SDL_RenderClear(renderer);
        }

        // Grid
        SDL_SetRenderDrawColor(renderer, 100, 110, 160, 80);
        for (int i = 0; i <= COLS; i++)
            SDL_RenderDrawLine(renderer, GRID_ORIGIN_X + i * CELL, GRID_ORIGIN_Y,
                               GRID_ORIGIN_X + i * CELL, GRID_ORIGIN_Y - ROWS * CELL);
        for (int j = 0; j <= ROWS; j++)
            SDL_RenderDrawLine(renderer, GRID_ORIGIN_X, GRID_ORIGIN_Y - j * CELL,
                               GRID_ORIGIN_X + COLS * CELL, GRID_ORIGIN_Y - j * CELL);

        // Axes
        drawArrow(renderer, {0, 0}, {COLS - 1, 0}, {240, 240, 255, 255}, 6);
        drawArrow(renderer, {0, 0}, {0, ROWS - 1}, {240, 240, 255, 255}, 6);
        renderText(renderer, font, "x", {220, 220, 255, 255},
                   GRID_ORIGIN_X + (COLS - 1) * CELL + 10, GRID_ORIGIN_Y + 5);
        renderText(renderer, font, "y", {220, 220, 255, 255},
                   GRID_ORIGIN_X - 20, GRID_ORIGIN_Y - ROWS * CELL - 5);

        // Basis vectors
        drawArrow(renderer, {0, 0}, u1_vis, {255, 120, 40, 255}, 8);
        drawArrow(renderer, {0, 0}, u2_vis, {60, 200, 255, 255}, 8);
        renderText(renderer, font, "u1", {255, 120, 40, 255},
                   GRID_ORIGIN_X + int(u1_vis.x * CELL) + 5,
                   GRID_ORIGIN_Y - int(u1_vis.y * CELL) - 25);
        renderText(renderer, font, "u2", {60, 200, 255, 255},
                   GRID_ORIGIN_X + int(u2_vis.x * CELL) + 5,
                   GRID_ORIGIN_Y - int(u2_vis.y * CELL) - 25);

        // y & its projections
        drawArrow(renderer, {0, 0}, y, {90, 255, 100, 255}, 5);
        drawPoint(renderer, y, {90, 255, 100, 255}, 8);
        if (showProj) {
            drawPoint(renderer, p1, {255, 120, 40, 255}, 8);
            drawPoint(renderer, p2, {60, 200, 255, 255}, 8);
        }

        // Instructions
        renderText(renderer, font,
                   (showProj ? "SPACE: hide projections" : "SPACE: show projections"),
                   {200, 200, 200, 180}, 10, 10);
        renderText(renderer, font, "ENTER: input projections",
                   {200, 200, 200, 180}, 10, 30);

        // Input overlay
        if (inputMode) {
            SDL_Rect ov{100, 220, WIDTH - 200, 140};
            SDL_SetRenderDrawColor(renderer, 30, 30, 50, 210);
            SDL_RenderFillRect(renderer, &ov);
            renderText(renderer, font,
                       stage == 1 ? "Enter y1 (x,y):" : "Enter y2 (x,y):",
                       {255, 255, 255, 255}, 120, 250);
            renderText(renderer, font, userInput + "|",
                       {255, 255, 200, 255}, 120, 290);
            if (!inputErr.empty()) {
                renderText(renderer, font, inputErr,
                           {255, 80, 80, 255}, 120, 330);
            }
        }

        // Result
        if (stage == 3) {
            SDL_Rect ov{WIDTH / 2 - 150, HEIGHT / 2 - 30, 300, 80};
            SDL_SetRenderDrawColor(renderer,
                winFlag ? 30 : 60, winFlag ? 120 : 30, winFlag ? 30 : 60, 200);
            SDL_RenderFillRect(renderer, &ov);
            renderText(renderer, font,
                winFlag ? "Correct!" : "Wrong!",
                winFlag ? SDL_Color{50, 255, 100, 255} : SDL_Color{255, 80, 80, 255},
                WIDTH / 2 - 50, HEIGHT / 2);
            SDL_RenderPresent(renderer);
            SDL_Delay(1500);

            if (!winFlag) goto start_game;  // restart if failed
            else break;                     // exit if passed
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    if (bgTex) SDL_DestroyTexture(bgTex);
    if (bgm) { Mix_HaltMusic(); Mix_FreeMusic(bgm); }
    if (clickSfx) Mix_FreeChunk(clickSfx);
    if (winSfx) Mix_FreeChunk(winSfx);
    if (font) TTF_CloseFont(font);
    Mix_HaltMusic();
}












