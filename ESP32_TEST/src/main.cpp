#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include "CAM.hpp"
#include "SDM.h"


void setup() 
{
    Serial.begin(115200);
    delay(1000); // Attendre que le port série soit prêt

    initSD();

    CAM camera;
    if(!camera.begin()) {
        Serial.println("Erreur d'initialisation de la caméra.");
        while(1);
    }

    camera.CaptureFrame("/capture.txt");
}

void loop() 
{

}