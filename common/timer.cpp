// common/timer.cpp
#include "timer.h"
#include <SDL2/SDL.h>

Timer::Timer() : startTicks(0), pausedTicks(0), started(false), paused(false) {}

void Timer::start() {
    started = true;
    paused = false;
    startTicks = SDL_GetTicks();
    pausedTicks = 0;
}

void Timer::stop() {
    started = false;
    paused = false;
    startTicks = 0;
    pausedTicks = 0;
}

void Timer::pause() {
    if (started && !paused) {
        paused = true;
        pausedTicks = SDL_GetTicks() - startTicks;
        startTicks = 0;
    }
}

void Timer::unpause() {
    if (paused) {
        paused = false;
        startTicks = SDL_GetTicks() - pausedTicks;
        pausedTicks = 0;
    }
}

unsigned int Timer::getTicks() const {
    if (started) {
        return paused ? pausedTicks : SDL_GetTicks() - startTicks;
    }
    return 0;
}

bool Timer::isStarted() const {
    return started;
}

bool Timer::isPaused() const {
    return paused && started;
}
