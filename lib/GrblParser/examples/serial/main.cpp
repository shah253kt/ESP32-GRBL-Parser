#ifndef UNIT_TEST

#include <Arduino.h>

#include "Config.h"
#include "SerialGrblParser.h"

#define GrblSerial Serial2

SerialGrblParser grblParser(GrblSerial);

void setup()
{
    Serial.begin(115200);
    GrblSerial.begin(115200);

    grblParser.onGCodeAboutToBeSent = [](std::string command)
    {
        Serial.printf("Sending command: %s\n", command.c_str());
    };

    grblParser.onResponseAboutToBeProcessed = [](std::string response)
    {
        Serial.printf("Got response: %s\n", response.c_str());
    };
}

void loop()
{
    grblParser.update();
    
    // auto success = grblParser.linearRapidPositioning({{Grbl::Axis::X, 100}, {Grbl::Axis::Y, -10}});        // G0 X100 Y-10
    // success = grblParser.linearInterpolationPositioning(1000, {{Grbl::Axis::X, 10}, {Grbl::Axis::Y, 50}}); // G1 F1000 X10 Y50
}

#endif // UNIT_TEST