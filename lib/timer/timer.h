#pragma once
#include <Arduino.h>
#include <array>
enum class TimerState
{
    IDLE,
    RUNNING,
    PAUSED
};

struct RGBColor
{
    uint8_t r, g, b;
};

class Timer
{
public:
    Timer();
    void start();
    void pause(RGBColor);
    void resume(RGBColor);
    void tick();
    void add(uint32_t, RGBColor);
    void stop();
    void before_stop();
    RGBColor getLastColor() const;
    uint32_t getRemainingTime() const;
    void set_duration(uint32_t);
    TimerState get_state();

private:
    TimerState state = TimerState::IDLE;
    uint32_t endMillis = 0;
    uint32_t remainingMilllis = 0;
    RGBColor lastLedColor = {0, 0, 0};
    uint32_t max_duration;
    RGBColor calcColor(uint32_t);
};
extern Timer timer;
