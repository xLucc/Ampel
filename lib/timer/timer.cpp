#include <timer.h>
#include <led.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <nfc.h>

#define DURATION 1200000 // 20 minutes in milliseconds

// Constructor initializes timer with maximum duration and LED array.
Timer::Timer()
{
    max_duration = DURATION;
}

// Starts the timer for a given duration and updates LEDs.
void Timer::start()
{
    state = TimerState::RUNNING;
    endMillis = millis() + max_duration;
    RGBColor color = calcColor(max_duration);
    RGBColor identifier = {0, 0, 255};
    show_color_wave(identifier);
    show_color(color);
}

// Pauses the timer, saving remaining time.
void Timer::pause(RGBColor color)
{
    if (state == TimerState::RUNNING)
    {
        remainingMilllis = endMillis - millis();
        state = TimerState::PAUSED;
        show_color_identifier(lastLedColor, color);
    }
}

// Resumes the timer from paused state.
void Timer::resume(RGBColor color)
{
    if (state == TimerState::PAUSED)
    {
        endMillis = millis() + remainingMilllis;
        state = TimerState::RUNNING;
        show_color_identifier(lastLedColor, color);
    }
}

// Adds extra time to the timer.
void Timer::add(uint32_t extra, RGBColor color)
{
    if (state == TimerState::IDLE)
    {
        return;
    }
    
    // Clamp extra time to max_duration
    if (extra > max_duration)
    {
        extra = max_duration;
    }

    unsigned long now = millis();

    if (state == TimerState::RUNNING)
    {
        // Calculate the new endMillis based on current time and extra time
        uint32_t timeLeft = (endMillis > now) ? (endMillis - now) : 0;
        // Clamp the new time left to max_duration
        uint32_t newLeft = min(timeLeft + extra, max_duration);
        // Update endMillis with the new time left
        endMillis = now + newLeft;
    }
    else if (state == TimerState::PAUSED)
    {
        // If paused, just add to remainingMilllis
        remainingMilllis = min(remainingMilllis + extra, max_duration);
    }
    // 
    show_color_identifier(lastLedColor, color);
}

// Updates timer state and LED color; should be called regularly.
void Timer::tick()
{
    if (state != TimerState::RUNNING)
    {
        return;
    }

    uint32_t left = (endMillis > millis()) ? (endMillis - millis()) : 0;

    RGBColor newColor = calcColor(left);

    if (newColor.r != lastLedColor.r ||
        newColor.g != lastLedColor.g ||
        newColor.b != lastLedColor.b)
    {
        lastLedColor = newColor;
        show_color(lastLedColor);
    }

    if (left == 0)
    {
        before_stop();
    }
}

// Prepares the timer for stopping, showing a warning color and waiting for user input.
void Timer::before_stop()
{
    bool killed = false;
    while (!killed)
    {
        show_color_identifier({255, 255, 0}, {0, 255, 255});
        killed = kill_timer();
    }
}

void Timer::stop()
{
    state = TimerState::IDLE;
    endMillis = 0;
    remainingMilllis = 0;
    show_color_bi_wave({255, 0, 0});
    show_color_identifier({255, 0, 0}, {0, 0, 255});
    clear_leds();
}

// Calculates LED color based on remaining time.

RGBColor Timer::calcColor(uint32_t left)
{
    
    if (left > max_duration)
    {
        left = max_duration;
    }
    float ratio = (float)left / (float)max_duration;
    
    RGBColor color;
    color.r = uint8_t(255 * (1.0f - ratio));
    color.g = uint8_t(255 * ratio);
    color.b = 0;
    return color;
}

RGBColor Timer::getLastColor() const
{
    return lastLedColor;
}

uint32_t Timer::getRemainingTime() const
{
    if (state == TimerState::RUNNING)
    {
        return (endMillis > millis()) ? (endMillis - millis()) : 0;
    }
    else if (state == TimerState::PAUSED)
    {
        return remainingMilllis;
    }
    else
    {
        return 0;
    }
}

void Timer::set_duration(uint32_t duration)
{
    max_duration = duration;
}

TimerState Timer::get_state()
{
    return state;
}
