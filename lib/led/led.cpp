#include <led.h>
#include <timer.h>
#include <FastLED.h>

#define DATA_PIN 4
#define BRIGHTNESS 100
#define NUM_LEDS 29
#define NUM_ROWS 5
#define NUM_COLS 5
#define SWAPPED_LEDS 4

CRGB leds[NUM_LEDS];

// This function initializes the LED strip by setting up the FastLED library.
// It configures the data pin, number of LEDs, and brightness level.
void led_setup()
{
    // Split the LED strip into two parts: the first SWAPPED_LEDS are in RGB order, the rest in GRB order.
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
}

// This function iterates over all available LEDs and assigns each one the color
// provided in the RGBColor structure. After setting the colors, it calls FastLED.show()
// to apply the changes to the physical LEDs.
void show_color(RGBColor color)
{
    for (int i = 0; i < SWAPPED_LEDS; i++)
    {
        leds[i] = CRGB(color.g, color.r, color.b);
    }

    for (int i = SWAPPED_LEDS; i < NUM_LEDS; i++)
    {
        leds[i] = CRGB(color.r, color.g, color.b);
    }

    FastLED.show();

    unsigned long start = millis();
    while (millis() - start < 50)
    {
        yield();
    }
}

// This function receives a color identifier and shows it on the LEDs.
// The identifier is a simple RGBColor value, provided by the NFC Chip, based on the command, its payload sends.
// It shows the color as baselayer and the identifier as moving pattern.
void show_color_identifier(RGBColor color, RGBColor identifier)
{
    // Set the base color for the first SWAPPED_LEDS
    for (int i = SWAPPED_LEDS; i < NUM_LEDS; i++)
    {
        leds[i] = CRGB(color.r, color.g, color.b);
    }
    FastLED.show();

    // Forward iteration through the rows
    for (int row = 0; row < NUM_ROWS; row++)
    {
        // Set identifier color
        for (int col = 0; col < NUM_COLS; col++)
        {
            int phy = SWAPPED_LEDS + row * NUM_COLS + col;
            leds[phy] = CRGB(identifier.r, identifier.g, identifier.b);
        }
        FastLED.show();

        // Delay to allow the identifier to be visible
        unsigned long start = millis();
        while (millis() - start < 75)
        {
            yield();
        }

    }

    // Backward iteration through the rows
    for (int row = NUM_ROWS - 1; row >= 0; row--)
    {
        // Set identifier color
        for (int col = 0; col < NUM_COLS; col++)
        {
            int phy = SWAPPED_LEDS + row * NUM_COLS + col;
            leds[phy] = CRGB(color.r, color.g, color.b);
        }
        FastLED.show();

        // Delay to allow the identifier to be visible
        unsigned long start = millis();
        while (millis() - start < 75)
        {
            yield();
        }

    }
}

// This function creates a wave effect on the LEDs.
void show_color_wave(RGBColor color)
{

    for (int row = 0; row < NUM_ROWS; row++)
    {
        // Set the color for the current row
        for (int col = 0; col < NUM_COLS; col++)
        {
            int phy = SWAPPED_LEDS + row * NUM_COLS + col;
            leds[phy] = CRGB(color.r, color.g, color.b);
        }
        FastLED.show();

        unsigned long start = millis();
        while (millis() - start < 75)
        {
            yield();
        }
    }
}

void clear_leds()
{
    FastLED.clear();
    FastLED.show();
}

// This function creates a bi-directional wave effect on the LEDs.
// It lights up 8 LEDs at a time, moving forward and then backward.
void show_color_bi_wave(RGBColor color)
{
    const int waveSize = 5;
    const unsigned long waveDelay = 50;
    // Calculate the number of LEDs that can be lit up in one wave
    int matrixLen = NUM_LEDS - SWAPPED_LEDS;
    // Calculate the number of waves needed to cover the entire matrix
    int numWaves = matrixLen / waveSize + (matrixLen % waveSize ? 1 : 0);

    // Forward iteration through the waves
    for (int w = 0; w < numWaves; w++)
    {

        // Clear the matrix LEDs
        FastLED.clear();

        // Set the wave LEDs
        for (int i = 0; i < waveSize; i++)
        {
            int phy = SWAPPED_LEDS + w * waveSize + i;
            if (phy < NUM_LEDS)
                leds[phy] = CRGB(color.r, color.g, color.b);
        }

        FastLED.show();
        unsigned long start = millis();
        while (millis() - start < waveDelay)
            yield();
    }

    // BAckward iteration through the waves
    for (int w = numWaves - 1; w >= 0; w--)
    {
        // Clear the matrix LEDs
        FastLED.clear();

        // Set the wave LEDs
        for (int i = 0; i < waveSize; i++)
        {
            int phy = SWAPPED_LEDS + w * waveSize + i;
            if (phy < NUM_LEDS)
                leds[phy] = CRGB(color.r, color.g, color.b);
        }

        FastLED.show();
        unsigned long start = millis();
        while (millis() - start < waveDelay)
            yield();
    }

    // Final delay to keep the last wave visible
    {
        unsigned long start = millis();
        while (millis() - start < waveDelay)
            yield();
    }
}


// This function creates a voltage warning effect on the LEDs.
// It lights up the Leds in a specific pattern to indicate low battery voltage.
void voltage_led()
{
    unsigned long startDelay = millis();
    while (millis() - startDelay < 20000) // 20 seconds
    {
        // Set the first SWAPPED_LEDS to red
        for (int i = 0; i < SWAPPED_LEDS; i++)
        {
            leds[i] = CRGB(0, 255, 0); // Red color for voltage warning
        }

        // Set the rest of the LEDs to red
        for (int i = SWAPPED_LEDS; i < NUM_LEDS; i++)
        {
            leds[i] = CRGB(255, 0, 0); // Red color for voltage warning
        }
        FastLED.show();

        // Wait for a short duration to create the effect
        unsigned long startDelay = millis();
        while (millis() - startDelay < 100)
        {
            yield();
        }

        // Set the leds to blue to indicate the voltage warning
        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = CRGB(0, 0, 255); // set to blue color
        }
        FastLED.show();

        // Wait for a short duration to create the effect
        unsigned long start = millis();
        while (millis() - start < 100)
        {
            yield();
        }
    }
}