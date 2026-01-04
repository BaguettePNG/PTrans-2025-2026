#ifndef __CAM_HPP__
#define __CAM_HPP__

#include <Arduino.h>
#include <Wire.h>
#include <SD.h>

// Adresse I2C de l'OV2640
#define OV2640_ADDR 0x30

// --- GPIO OV7670 (Pins de l'utilisateur) ---
#define SIOD_GPIO   21 // SDA
#define SIOC_GPIO   22 // SCL

#define D0_GPIO     4   // D0
#define D1_GPIO     5   // D1
#define D2_GPIO     18  // D2
#define D3_GPIO     19  // D3
#define D4_GPIO     36  // D4
#define D5_GPIO     39  // D5
#define D6_GPIO     34  // D6
#define D7_GPIO     35  // D7

#define VSYNC_GPIO  32  // VSYNC 
#define HREF_GPIO   33  // HREF
#define PCLK_GPIO   25  // PCLK

#define PIXEL_PER_LINE_UXGA 1600
#define NB_LINES_UXGA 1200

class CAM
{
    private:
        
    public:
        CAM();
        void CaptureFrame(const char* filename);
        void CaptureFrameBMP(const char* filename) 

    private : 
        void writeReg(uint8_t reg, uint8_t val); // Ecriture registre I2C
        void RegisterInit();                     // Initialisation des registres de la caméra
        uint8_t readByteFast();                  // Lecture rapide d'un octet de données
        int readGPIO(uint8_t pin);               // Lecture rapide d'un GPIO
};

int readGPIO(uint8_t pin)
{
    if (pin < 32) 
    {
        return (GPIO.in >> pin) & 1;
    } 
    else if (pin < 40) 
    {
        return (GPIO.in1.data >> (pin - 32)) & 1;
    }
    return 0; 
}

uint8_t CAM::readByteFast()
{    
    uint8_t value = 0;

    value |= (readGPIO(D0_GPIO) << 0);
    value |= (readGPIO(D1_GPIO) << 1);
    value |= (readGPIO(D2_GPIO) << 2);
    value |= (readGPIO(D3_GPIO) << 3);
    value |= (readGPIO(D4_GPIO) << 4);
    value |= (readGPIO(D5_GPIO) << 5);
    value |= (readGPIO(D6_GPIO) << 6);
    value |= (readGPIO(D7_GPIO) << 7);

    return value;   
}

void CAM::writeReg(uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(OV2640_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
    delayMicroseconds(100);
}

void CAM::RegisterInit() 
{
    // 1. Reset complet du capteur
    writeReg(0xFF, 0x01); // Sélection Bank 1 (Sensor)
    writeReg(0x12, 0x80); // Reset matériel via SCCB [cite: 83]
    delay(100);

    // 2. Configuration Résolution UXGA (1600x1200)
    // On s'assure d'être sur la Bank 1 pour les timings capteur
    writeReg(0xFF, 0x01); 
    writeReg(0x11, 0x01); // CLKRC : Horloge interne (Prescaler), 15 fps en UXGA [cite: 55, 101]
                          // Augmenter la valeur si image décaler pour ralentir la caméra
    writeReg(0x12, 0x00); // COM7 : Mode UXGA, Format Raw (par défaut avant DSP) [cite: 67]
    
    // Fenêtrage par défaut pour 1600x1200 [cite: 32]
    // (Les registres HREFST, HREFEND, VSTRT, VEND sont généralement aux valeurs par défaut pour UXGA)

    // 3. Configuration Format de sortie RGB565 et Qualité (Bank 0)
    writeReg(0xFF, 0x00); // Sélection Bank 0 (DSP) [cite: 114]
    
    // Activer le DSP et configurer le format de sortie
    writeReg(0x05, 0x00); // R_BYPASS : Utiliser le DSP (0: DSP, 1: Bypass) 
    
    // Configuration du format RGB565 sur le port 8-bit [cite: 231]
    // Note : COM7 en Bank 0 (0x12) est souvent utilisé pour définir le format final
    writeReg(0x12, 0x09); // Exemple type : Output RGB565
    
    // 4. Optimisation de la Qualité Image (DSP)
    // Activation des fonctions de traitement interne [cite: 64, 211]
    writeReg(0x44, 0x0C); // Qs : Facteur de mise à l'échelle de la quantification par défaut 
    
    /* Pour une qualité maximale, le DSP gère automatiquement :
       - Matrice RGB (élimination du cross-talk) [cite: 64]
       - Netteté / Edge enhancement [cite: 211]
       - Correction de Gamma programmable [cite: 231]
       - Débruitage et annulation de pixels blancs [cite: 212]
    */

    // 5. Configuration finale du port vidéo (Synchronisation)
    writeReg(0xFF, 0x01); // Retour Bank 1
    writeReg(0x15, 0x00); // COM10 : PCLK actif sur front descendant (standard) [cite: 87]
}

CAM::CAM()
{
    // Camera GPIO setup
    pinMode(VSYNC_GPIO, INPUT);
    pinMode(HREF_GPIO, INPUT);
    pinMode(PCLK_GPIO, INPUT);

    pinMode(D0_GPIO, INPUT);
    pinMode(D1_GPIO, INPUT);
    pinMode(D2_GPIO, INPUT);
    pinMode(D3_GPIO, INPUT);
    pinMode(D4_GPIO, INPUT);
    pinMode(D5_GPIO, INPUT);
    pinMode(D6_GPIO, INPUT);
    pinMode(D7_GPIO, INPUT);

    // Initialisation I2C
    Wire.begin(SIOD_GPIO, SIOC_GPIO);
    Wire.setClock(100000);  // 100kHz pour l'initialisation
    delay(100);

    // Register camera initialization would go here
    RegisterInit();
}

void CAM::CaptureFrame(const char* filename) 
{
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("Erreur : Impossible d'ouvrir le fichier SD");
        return;
    }

    Serial.println("Attente du signal VSYNC...");

    // 1. Attendre la fin du VSYNC actuel (s'il est déjà en cours)
    while (readGPIO(VSYNC_GPIO) == 0);
    // 2. Attendre le début d'une nouvelle trame (VSYNC passe à 0)
    while (readGPIO(VSYNC_GPIO) == 1);

    // UXGA : 1600 pixels par ligne, 1200 lignes. 
    // RGB565 : 2 octets par pixel.
    for (int y = 0; y < NB_LINES_UXGA; y++) 
    {
        // Attendre que HREF devienne haut (début de ligne)
        while (readGPIO(HREF_GPIO) == 0);

        for (int x = 0; x < PIXEL_PER_LINE_UXGA; x++) 
        {
            // --- Premier Octet (High Byte : RRRRRGGG) ---
            while (readGPIO(PCLK_GPIO) == 0); // Attendre front montant PCLK
            uint8_t byte1 = readByteFast();
            while (readGPIO(PCLK_GPIO) == 1); // Attendre front descendant PCLK

            // --- Deuxième Octet (Low Byte : GGGBBBBB) ---
            while (readGPIO(PCLK_GPIO) == 0); 
            uint8_t byte2 = readByteFast();
            while (readGPIO(PCLK_GPIO) == 1);

            // Écriture sur la carte SD
            file.write(byte1);
            file.write(byte2);
        }

        // Attendre que HREF devienne bas (fin de ligne)
        while (readGPIO(HREF_GPIO) == 1);
    }

    file.close();
    Serial.println("Capture UXGA RGB565 terminée.");
}

void CAM::CaptureFrameBMP(const char* filename) 
{
    // 1. Préparation du Header BMP (UXGA 1600x1200, 24-bit)
    uint32_t width = 1600;
    uint32_t height = 1200;
    uint32_t fileSize = 54 + (width * height * 3);
    
    uint8_t bmpHeader[54] = {
        0x42, 0x4D,             // Signature "BM"
        (uint8_t)(fileSize), (uint8_t)(fileSize >> 8), (uint8_t)(fileSize >> 16), (uint8_t)(fileSize >> 24),
        0x00, 0x00, 0x00, 0x00, // Réservé
        0x36, 0x00, 0x00, 0x00, // Offset data (54)
        0x28, 0x00, 0x00, 0x00, // Taille header info (40)
        (uint8_t)(width), (uint8_t)(width >> 8), (uint8_t)(width >> 16), (uint8_t)(width >> 24),
        (uint8_t)(height), (uint8_t)(height >> 8), (uint8_t)(height >> 16), (uint8_t)(height >> 24),
        0x01, 0x00,             // Plans (1)
        0x18, 0x00,             // Bits par pixel (24 bits pour RGB888)
        0x00, 0x00, 0x00, 0x00, // Compression (0 = aucune)
        0x00, 0x00, 0x00, 0x00, // Taille image (0 si pas de compression)
        0x00, 0x00, 0x00, 0x00, // Résolution H
        0x00, 0x00, 0x00, 0x00, // Résolution V
        0x00, 0x00, 0x00, 0x00, // Couleurs palette
        0x00, 0x00, 0x00, 0x00  // Couleurs importantes
    };

    File file = SD.open(filename, FILE_WRITE);
    if (!file) return;
    file.write(bmpHeader, 54);

    // 2. Buffer de ligne pour s'affranchir de la latence SD
    // On stocke les pixels convertis en RGB888 (3 octets par pixel)
    uint8_t* lineBuffer = (uint8_t*)malloc(width * 3);
    if (!lineBuffer) return;

    // 3. Synchronisation VSYNC
    while (readGPIO(VSYNC_GPIO) == 0);
    while (readGPIO(VSYNC_GPIO) == 1);

    // Note : Le format BMP stocke les images de BAS en HAUT. 
    // Pour un code simple ici, nous lisons de haut en bas (image sera inversée verticalement)
    for (int y = 0; y < height; y++) {
        while (readGPIO(HREF_GPIO) == 0); // Début de ligne

        for (int x = 0; x < width; x++) {
            // Lecture Octet 1 (RGB565 : RRRRRGGG)
            while (readGPIO(PCLK_GPIO) == 0);
            uint8_t b1 = readByteFast();
            while (readGPIO(PCLK_GPIO) == 1);

            // Lecture Octet 2 (RGB565 : GGGBBBBB)
            while (readGPIO(PCLK_GPIO) == 0);
            uint8_t b2 = readByteFast();
            while (readGPIO(PCLK_GPIO) == 1);

            // Conversion RGB565 vers RGB888 (pour le BMP 24-bit)
            // BMP utilise l'ordre BGR
            lineBuffer[x * 3 + 2] = (b1 & 0xF8);                    // Rouge
            lineBuffer[x * 3 + 1] = ((b1 << 5) | (b2 >> 3)) & 0xFC; // Vert
            lineBuffer[x * 3 + 0] = (b2 << 3);                      // Bleu
        }

        // Écriture de la ligne complète sur la SD d'un seul coup
        file.write(lineBuffer, width * 3);
        
        while (readGPIO(HREF_GPIO) == 1); // Fin de ligne
    }

    free(lineBuffer);
    file.close();
    Serial.println("Capture BMP 24-bit terminée.");
}

#endif