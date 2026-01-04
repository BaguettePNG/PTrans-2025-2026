#include <Arduino.h>
#include <Wire.h>
#include "CAM.hpp"
#include "SDM.h"

CAM cam; // L'objet est créé mais il est "endormi" (pas de crash au boot)

void setup() {
    Serial.begin(115200);
    delay(1000); // Laisse l'électronique se stabiliser

    // 1. Initialisation I2C CENTRALISÉE
    Wire.begin(21, 22); 
    Wire.setClock(100000);
    delay(200);

    // 2. Initialisation SD
    initSD();

    // 3. Initialisation Caméra via la nouvelle méthode begin()
    if (!cam.begin()) {
        Serial.println("Echec Caméra !");
        while(1);
    }

    Serial.println("Capture...");
    cam.CaptureFrameBMP("/photo.bmp");
    Serial.println("Fini.");
}

void loop() {}