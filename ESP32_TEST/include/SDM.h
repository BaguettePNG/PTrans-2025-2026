#ifndef __SDM_HPP__
#define __SDM_HPP__

#include <SPI.h>
#include <SD.h>

SPIClass hspi(HSPI);

#define SD_SCK  14
#define SD_MISO 12
#define SD_MOSI 13
#define SD_CS   15

void initSD()
{
    // CS doit être configuré avant tout
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);  // désactivé

    // Init bus HSPI
    hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    if (!SD.begin(SD_CS, hspi)) {
        Serial.println("Erreur SD sur HSPI");
    } else {
        Serial.println("SD OK sur HSPI");
    }
}





#endif