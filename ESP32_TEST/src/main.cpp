#include <Arduino.h>
#include "CAM.hpp"
#include "SDM.h"

// Initialisation de la caméra OV7670
CAM cam;

void setup()
{

    Serial.begin(115200);
    delay(1000);

    // Init SD
    initSD();

  // Init caméra
    if(cam.begin())
    {
        Serial.println("Camera OK !");
    }
    else
    {
        Serial.println("Erreur camera");
        while(1);
    }

    // Capture test
    Serial.println("Capture en cours...");
    if(cam.captureToSD("/img.raw"))
    {
        Serial.println("Capture terminée avec succès !");
    }
    else
    {
        Serial.println("Erreur capture !");
    }
}

void loop()
{
    // Rien à faire pour le moment
}