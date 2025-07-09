#include "rsa_game.h"
#include "../../common/utils.h"
#include "../../common/game_state.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

long long mod_exp(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result;
}

std::string decryptRSA(const std::string& encryptedStr, long long e, long long n) {
    std::stringstream ss(encryptedStr);
    std::string token, result;
    while (ss >> token) {
        long long cipher = std::stoll(token);
        char decryptedChar = static_cast<char>(mod_exp(cipher, e, n));
        result += decryptedChar;
    }
    return result;
}
void runRSAGame(SDL_Renderer* renderer) {
    TTF_Font* font = TTF_OpenFont("assets/fonts/impact.ttf", 24);
    if (!font) return;

    SDL_Texture* bg         = loadTexture(renderer, "assets/images/rsa_background.png");
    SDL_Texture* decryptor  = loadTexture(renderer, "assets/images/decryptor.png");

    Mix_Music* music = Mix_LoadMUS("assets/audio/rsa_background.mp3");
    if (music) Mix_PlayMusic(music, -1);

    Mix_Chunk* correct = Mix_LoadWAV("assets/audio/correct.mp3");
    Mix_Chunk* wrong   = Mix_LoadWAV("assets/audio/wrong.mp3");

    std::string inputN, inputE, inputEnc, result;
    enum Focus { FOCUS_N, FOCUS_E, FOCUS_ENC } currentFocus = FOCUS_N;

    SDL_StartTextInput();
    SDL_Event e;
    bool running = true;
    bool solved = false;
    bool showingInfo = false;

    SDL_Rect rectN   = {200, 50, 500, 40};
    SDL_Rect rectD   = {200, 120, 500, 40};
    SDL_Rect rectEnc = {200, 190, 500, 40};
    SDL_Rect button  = {350, 260, 150, 40};
    SDL_Rect infoBtn = {600, 20, 180, 40};
    SDL_Rect backBtn = {20, 20, 100, 40};

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                break;
            }

            int mx = e.button.x, my = e.button.y;

            if (!showingInfo) {
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    if (mx >= rectN.x && mx <= rectN.x + rectN.w &&
                        my >= rectN.y && my <= rectN.y + rectN.h)
                        currentFocus = FOCUS_N;
                    else if (mx >= rectD.x && mx <= rectD.x + rectD.w &&
                             my >= rectD.y && my <= rectD.y + rectD.h)
                        currentFocus = FOCUS_E;
                    else if (mx >= rectEnc.x && mx <= rectEnc.x + rectEnc.w &&
                             my >= rectEnc.y && my <= rectEnc.y + rectEnc.h)
                        currentFocus = FOCUS_ENC;
                    else if (mx >= button.x && mx <= button.x + button.w &&
                             my >= button.y && my <= button.y + button.h) {
                        try {
                            long long n = std::stoll(inputN);
                            long long e = std::stoll(inputE);
                            if (n == 2537 && e == 13 && inputEnc == "2081 2182 2024") {
                                result = "Door Opened";
                                solved = true;
                                if (correct) Mix_PlayChannel(-1, correct, 0);
                                rsaSolved = true;  // RSA game solved, unlock Door 3
                         
    SDL_Rect button  = {350, 260, 150, 40};   } else {
                                result = "Incorrect. Try again.";
                                if (wrong) Mix_PlayChannel(-1, wrong, 0);
                            }
                        } catch (...) {
                            result = "Invalid input.";
                            if (wrong) Mix_PlayChannel(-1, wrong, 0);
                        }
                    } else if (mx >= infoBtn.x && mx <= infoBtn.x + infoBtn.w &&
                               my >= infoBtn.y && my <= infoBtn.y + infoBtn.h) {
                        showingInfo = true;
                    }
                }

                if (e.type == SDL_TEXTINPUT) {
                    if (currentFocus == FOCUS_N) inputN += e.text.text;
                    else if (currentFocus == FOCUS_E) inputE+= e.text.text;
                    else if (currentFocus == FOCUS_ENC) inputEnc += e.text.text;
                }

                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKSPACE) {
                    if (currentFocus == FOCUS_N && !inputN.empty()) inputN.pop_back();
                    else if (currentFocus == FOCUS_E && !inputE.empty()) inputE.pop_back();
                    else if (currentFocus == FOCUS_ENC && !inputEnc.empty()) inputEnc.pop_back();
                }

                if (solved && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                    running = false;
                }

            } else {
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    if (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                        my >= backBtn.y && my <= backBtn.y + backBtn.h) {
                        showingInfo = false;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (showingInfo) {
            if (decryptor) SDL_RenderCopy(renderer, decryptor, nullptr, nullptr);

            SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
            SDL_RenderFillRect(renderer, &backBtn);
            SDL_Color white = {255,255,255,255};
            SDL_Rect r;
            SDL_Texture* txt = renderText(renderer, font, "Back", white, r);
            r.x = backBtn.x + 20; r.y = backBtn.y + 8;
            SDL_RenderCopy(renderer, txt, nullptr, &r);
            SDL_DestroyTexture(txt);
            SDL_RenderPresent(renderer);
            continue;
        }

        if (bg) SDL_RenderCopy(renderer, bg, nullptr, nullptr);

        SDL_Color white = {255, 255, 255, 255};
        SDL_Color highlight = {50, 255, 50, 255};
        SDL_Color red = {255, 60, 60, 255};

        SDL_Rect r;

        auto drawInput = [&](const std::string& label, const std::string& val, SDL_Rect rect, bool focused) {
           SDL_Texture* txt = renderText(renderer, font, label, white, r);
if (txt) {
    r.x = 50; r.y = rect.y + 10;
    SDL_RenderCopy(renderer, txt, nullptr, &r);
    SDL_DestroyTexture(txt);
}


            SDL_SetRenderDrawColor(renderer, focused ? highlight.r : 180,
                                   focused ? highlight.g : 180,
                                   focused ? highlight.b : 180, 255);
            SDL_RenderDrawRect(renderer, &rect);

           SDL_Texture* vtxt = renderText(renderer, font, val, white, r);
if (vtxt) {
    r.x = rect.x + 10; r.y = rect.y + 10;
    SDL_RenderCopy(renderer, vtxt, nullptr, &r);
    SDL_DestroyTexture(vtxt);
}
        };

        drawInput("Enter n:", inputN, rectN, currentFocus == FOCUS_N);
        drawInput("Enter e:", inputE, rectD, currentFocus == FOCUS_E);
        drawInput("Encrypted text:", inputEnc, rectEnc, currentFocus == FOCUS_ENC);

        // Decrypt button
        SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
        SDL_RenderFillRect(renderer, &button);
       SDL_Texture* btxt = renderText(renderer, font, "Decrypt", {30,30,30,255}, r);
if (btxt) {
    r.x = button.x + 20; r.y = button.y + 8;
    SDL_RenderCopy(renderer, btxt, nullptr, &r);
    SDL_DestroyTexture(btxt);
}


        // Info button
        SDL_SetRenderDrawColor(renderer, 30, 144, 255, 255);
        SDL_RenderFillRect(renderer, &infoBtn);
        SDL_Texture* itxt = renderText(renderer, font, "Decryptor Info", white, r);
        r.x = infoBtn.x + 10; r.y = infoBtn.y + 10;
        SDL_RenderCopy(renderer, itxt, nullptr, &r);
        SDL_DestroyTexture(itxt);

        // Result
        SDL_Color resultColor = (result == "Door Opened") ? highlight : red;
        SDL_Texture* rtxt = renderText(renderer, font, result, resultColor, r);
        r.x = 50; r.y = 330;
        SDL_RenderCopy(renderer, rtxt, nullptr, &r);
        SDL_DestroyTexture(rtxt);

        if (solved) {
            SDL_Texture* stxt = renderText(renderer, font, "Press SPACE to return", white, r);
            r.x = 50; r.y = 380;
            SDL_RenderCopy(renderer, stxt, nullptr, &r);
            SDL_DestroyTexture(stxt);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    if (bg) SDL_DestroyTexture(bg);
    if (decryptor) SDL_DestroyTexture(decryptor);
    if (font) TTF_CloseFont(font);
    if (music) {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
    }
    if (correct) Mix_FreeChunk(correct);
    if (wrong) Mix_FreeChunk(wrong);
}
