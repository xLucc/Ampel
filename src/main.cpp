#include <Arduino.h>
#include <nfc.h>
#include <webserver.h>
#include <led.h>
#include <battery.h>
#include <WiFi.h>

unsigned long start;

void setup()
{
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH); // Ensure pin 3 is low initially
  pinMode(2, INPUT);
  delay(5000); // Allow time for Serial to initialize
  nfc_setup(); // Initialize NFC hardware and libraries
  led_setup(); // Initialize LED hardware
  WiFi.mode(WIFI_AP);
  String ssid = "Klausurampel_" + WiFi.macAddress();
  WiFi.softAP(ssid.c_str());

  IPAddress IP = WiFi.softAPIP();
  webserver_setup(); // Initialize web server
  start = millis();
}

void loop()
{
  get_battery_voltage();   // Read the battery voltage
  store_battery_voltage(); // Store the battery voltage in SPIFFS

  if (millis() - start >= 80000)
  {
     if (critical_battery_voltage())
     {
       digitalWrite(3, LOW); // Set pin 3 low if battery is critical
     }
  }

  nfc_loop();
}
