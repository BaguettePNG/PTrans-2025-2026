#ifndef __SDM_H__
#define __SDM_H__

#include <SPI.h>
#include <SD.h>

// Définitions des broches SD pour HSPI
#define SD_SCK  14
#define SD_MISO 12
#define SD_MOSI 13
#define SD_CS   15

// Vitesse SPI : 40MHz est généralement le maximum stable pour l'ESP32 en SPI.
// Si cela échoue, descendez à 20000000 (20MHz).
#define SPI_SPEED 40000000 

SPIClass hspi(HSPI);

/**
 * @brief Initialise le bus SPI à haute vitesse.
 */
void initSD()
{
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    // Initialisation HSPI
    hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    // Tentative à 40MHz (Vitesse maximale)
    if (!SD.begin(SD_CS, hspi, SPI_SPEED)) { 
        Serial.println("Échec à 40MHz, tentative à 20MHz...");
        if (!SD.begin(SD_CS, hspi, 20000000)) {
            Serial.println("Erreur critique SD. Vérifiez la longueur des câbles.");
        } else {
            Serial.println("SD OK à 20MHz.");
        }
    } else {
        Serial.println("SD OK à 40MHz (Vitesse Max).");
    }
}

/**
 * @brief Sauvegarde ultra-rapide d'un buffer.
 */
void writeFile(const char * path, uint8_t * buf, size_t len) {
    // Note : On retire le printf pour gagner quelques millisecondes
    File file = SD.open(path, FILE_WRITE);
    if(!file){
        return;
    }

    // L'astuce : file.write(buf, len) est déjà optimisé pour les gros buffers 
    // car la librairie SD utilise le mode de transfert par bloc du driver SPI.
    size_t i = file.write(buf, len);
    
    file.close(); 
    
    if(i != len) {
        Serial.println("Erreur d'écriture (Disque plein ou retrait)");
    }
}

#endif // __SDM_H__