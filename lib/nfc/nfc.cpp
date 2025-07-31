#include <Arduino.h>
#include "nfc.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <timer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#define SDA 6
#define SCL 7
#define RESET_PIN 21
#define CYCLE_TIME 200 // milliseconds
#define TIMEOUT 1000   // milliseconds
#define BLOCK 8


Adafruit_PN532 nfc(-1, RESET_PIN, &Wire);
uint8_t uid[7];                                        // Buffer for UID
uint8_t uidLength;                                     // Length of UID
uint8_t Key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Default Mifare Classic key
uint8_t data[16];                                      // Buffer for NFC            
Timer timer;                                           // Timer for command execution

void nfc_setup()
{
    Wire.begin(SDA, SCL);
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW);  // Ensure NFC reset pin is low initially
    delay(10);                     // Wait for NFC module to reset
    digitalWrite(RESET_PIN, HIGH); // Set NFC reset pin high to wake up the module
    delay(10);                     // Allow time for the NFC module to initialize
    nfc.begin();
    nfc.SAMConfig();
    load_timer();                  // Load timer configuration from SPIFFS
}

void nfc_loop()
{

    timer.tick(); 
    static unsigned long lastCycleTime = 0;
    unsigned long now = millis();

    nfc.reset();
    // Check if enough time has passed since the last cycle
    if (now - lastCycleTime < CYCLE_TIME)
    {
        return;
    }
    lastCycleTime = now;

    // Check if a card is present
    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, TIMEOUT))
    {
        return;
    }

    // Authenticate the card using the default key
    if (!nfc.mifareclassic_AuthenticateBlock(uid, uidLength, BLOCK, 0, Key))
    {
        return;
    }
    
    // Read the data from the NFC block
    if (!nfc.mifareclassic_ReadDataBlock(BLOCK, data))
    {
        return;
    }

    // Map the data to a COMMAND struct and execute it
    COMMAND cmd = cpy();
    exec(cmd);
}

// Copy the data from the NFC block to a COMMAND struct
// This function extracts the command data from the NFC block and returns it as a COMMAND struct.
COMMAND cpy()
{
    COMMAND cmd;
    cmd.funcion = data[0];
    cmd.timeValue = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
    cmd.red = data[5];
    cmd.green = data[6];
    cmd.blue = data[7];
    cmd.checksum = data[8];
    return cmd;
}

void exec(COMMAND cmd)
{
    Serial.println("Executing command...");
    switch (cmd.funcion)
    {
    // Start or stop the timer based on the command
    case 0x01:
        if (timer.get_state() == TimerState::IDLE)
        {
            timer.start();
        }
        else if (timer.get_state() == TimerState::RUNNING)
        {
            timer.stop();
        }

        break;

    // Start or stop the timer based on the command
    case 0x02:
        if (timer.get_state() == TimerState::PAUSED)
        {
            timer.resume({cmd.red, cmd.green, cmd.blue});
        }
        else if (timer.get_state() == TimerState::RUNNING)
        {
            timer.pause({cmd.red, cmd.green, cmd.blue});
        }
        break;

    // Admin Card
    case 0x03:
        timer.stop();
        break;

    // Set the timer duration
    case 0x04:
        // Set the timer duration
        timer.add(cmd.timeValue, {cmd.red, cmd.green, cmd.blue});
        break;

    // Kill the ESP
    case 0x05:
        digitalWrite(3, LOW);
        break;

    default:
        break;
    }
}

bool kill_timer()
{
    static unsigned long lastCycleTime = 0;
    unsigned long now = millis();

    // Check if enough time has passed since the last cycle
    if (now - lastCycleTime < CYCLE_TIME)
    {
        return false;
    }
    lastCycleTime = now;

    // If the timer is in STOPPING state, we can proceed to kill it
    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, TIMEOUT))
    {
        return false;
    }

    // If the UID is not valid, return false
    if (!nfc.mifareclassic_AuthenticateBlock(uid, uidLength, BLOCK, 0, Key))
    {
        return false;
    }

    // If the block cannot be read, return false
    if (!nfc.mifareclassic_ReadDataBlock(BLOCK, data))
    {
        return false;
    }

    if (data[0] == 0x01)
    {
        timer.stop();
        return true;

    } else if (data[0] == 0x05) {
        digitalWrite(3, LOW);
    }


    return false;
}


// Load the timer configuration from SPIFFS
// This function reads the timer duration from a JSON file and sets it in the timer.
void load_timer() {
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS konnte nicht gemountet werden");
        return;
    }
    File file = SPIFFS.open("/config.json", "r");
    if (!file)
    {
        Serial.println("Failed to open config file for reading");
        return;
    }

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        Serial.println("Failed to parse config file");
        return;
    }

    // Check if the "duration" key exists in the JSON document
    uint32_t duration = doc["duration"].as<uint32_t>();
    Serial.println("Loaded timer duration: " + String(duration));
    timer.set_duration(duration * 60000); // Convert minutes to milliseconds
}