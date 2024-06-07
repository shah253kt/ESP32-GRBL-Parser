#ifndef UNIT_TEST

#include <Arduino.h>

#include "Config.h"
#include "WebsocketGrblParser.h"

WebsocketGrblParser grblParser(FLUIDNC_IP, FLUIDNC_WEBSOCKET_PORT, "/");

bool connectToWifi();

void setup()
{
    Serial.begin(115200);

    if (!connectToWifi())
    {
        Serial.println("Failed to connect to WiFi. Restarting ESP.");
        esp_restart();
    }

    grblParser.onGCodeAboutToBeSent = [](std::string command)
    {
        Serial.printf("Sending command: %s\n", command.c_str());
    };

    grblParser.onResponseAboutToBeProcessed = [](std::string response)
    {
        Serial.printf("Got response: %s\n", response.c_str());
    };

    grblParser.connect();
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi disconnected. Restarting ESP.");
        esp_restart();
    }

    grblParser.update();
    // auto success = grblParser.linearRapidPositioning({{Grbl::Axis::X, 100}, {Grbl::Axis::Y, -10}});        // G0 X100 Y-10
    // success = grblParser.linearInterpolationPositioning(1000, {{Grbl::Axis::X, 10}, {Grbl::Axis::Y, 50}}); // G1 F1000 X10 Y50
}

bool connectToWifi()
{
    Serial.println("Connecting to Wifi");

    // Connect to wifi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Wait some time to connect to wifi
    for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++)
    {
        Serial.print(".");
        delay(500);
    }

    return WiFi.status() == WL_CONNECTED;
}

#endif // UNIT_TEST