#ifndef __SDM_H__
#define __SDM_H__

#include <SPI.h>
#include <SD.h>

// Définitions des broches SD pour HSPI (selon votre configuration)
#define SD_SCK  14
#define SD_MISO 12
#define SD_MOSI 13
#define SD_CS   15

SPIClass hspi(HSPI);

/**
 * @brief Initialise le bus SPI (HSPI) et la carte SD.
 */
void initSD()
{
    // CS doit être configuré avant tout
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    if (!SD.begin(SD_CS, hspi, 4000000)) { 
        Serial.println("Erreur SD sur HSPI");
        if (!SD.begin(SD_CS, hspi, 1000000)) {
            Serial.println("Erreur SD même à 1MHz. Vérifiez le câblage.");
        } else {
            Serial.println("SD OK sur HSPI à 1MHz.");
        }
    } else {
        Serial.println("SD OK sur HSPI à 4MHz.");
    }
}

/**
 * @brief Sauvegarde un buffer d'image sur la carte SD.
 * @param path Le nom du fichier (ex: "/capture.bmp").
 * @param buf Le pointeur vers le buffer de l'image.
 * @param len La longueur du buffer en octets.
 */
void writeFile(const char * path, uint8_t * buf, size_t len) {
    Serial.printf("Écriture du fichier : %s\n", path);

    File file = SD.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Erreur : échec de l'ouverture du fichier en écriture.");
        return;
    }

    if(file.write(buf, len) == len){
        Serial.println("Fichier enregistré avec succès.");
    } else {
        Serial.println("Erreur : échec de l'écriture du fichier.");
    }
    file.close();
}

#endif // __SDM_H__