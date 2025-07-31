#include "battery.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <led.h>

#define RING_BUFFER_SIZE 32
#define BATTERY_VOLTAGE_PIN 2
#define THRESHOLD 3.6
#define MAX_TRIGGER 15

uint32_t ringBuffer[RING_BUFFER_SIZE];
uint32_t sum = 0;
int ringBufferIndex = 0;
int trigger = 0;
float batteryVoltage = 0.0;
int ring_ticks = 0;

void store_battery_voltage()
{
    File file = SPIFFS.open("/config.json", "r");

    StaticJsonDocument<256> doc;

    if (file)
    {
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        if (error)
        {
            doc.clear();
        }
    }
    doc["battery_voltage"] = batteryVoltage;
    file = SPIFFS.open("/config.json", "w");
    if (file)
    {
        serializeJsonPretty(doc, file);
        file.close();
    }
}

void get_battery_voltage()
{  
    if(ring_ticks < RING_BUFFER_SIZE)
    {
        ring_ticks++;
    }
    else
    {
        ring_ticks = RING_BUFFER_SIZE;
    }
    int raw = analogReadMilliVolts(BATTERY_VOLTAGE_PIN);
    sum = sum - ringBuffer[ringBufferIndex] + raw;              // Subtract the oldest value
    ringBuffer[ringBufferIndex] = raw;                          // Store the new value
    ringBufferIndex = (ringBufferIndex + 1) % RING_BUFFER_SIZE; // Move to the next index
    batteryVoltage = ((sum / ring_ticks) / 1000.0) * 2 + 0.2;  // Calculate average voltage
    Serial.println("Voltage: " + String(batteryVoltage) + "V");
}

bool critical_battery_voltage()
{
    if (batteryVoltage < THRESHOLD)
    {
        trigger++;
    }

    if (trigger >= MAX_TRIGGER)
    {
        voltage_led();
        return true; // Critical battery voltage reached
    }

    return false; // Battery voltage is normal
}