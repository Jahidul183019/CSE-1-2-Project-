// common/timer.h
#ifndef TIMER_H
#define TIMER_H

class Timer {
public:
    Timer();

    void start();
    void stop();
    void pause();
    void unpause();

    unsigned int getTicks() const;
    bool isStarted() const;
    bool isPaused() const;

private:
    unsigned int startTicks;
    unsigned int pausedTicks;

    bool started;
    bool paused;
};

#endif // TIMER_H
