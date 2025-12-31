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

class CAM
{
    private:
        
    public:
        CAM();
        void CaptureFrame(const char* filename);

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

void CAM::RegisterInit() {
    // Reset complet
    writeReg(0xFF, 0x01);
    writeReg(0x12, 0x80);
    delay(100);

    // Configuration simplifiée pour JPEG 480p (DVGA 720x480)
    // Note : Pour un code de production, une table complète est normalement nécessaire
    writeReg(0xFF, 0x00);
    writeReg(0x2C, 0xFF); // Prescaler
    writeReg(0x2E, 0xDF); 
    
    writeReg(0xFF, 0x01);
    writeReg(0x11, 0x01); // Clock divider
    writeReg(0x12, 0x00); // Mode normal (UXGA si 0x40, SVGA si 0x00)
    
    // Switch vers DSP pour le scaling
    writeReg(0xFF, 0x00);
    writeReg(0x05, 0x01); // Activer JPEG
    writeReg(0xE0, 0x04);
    writeReg(0xC0, 0xC8); // Taille Horizontale (720 -> 0x2D0)
    writeReg(0xC1, 0x1E); // Taille Verticale (480 -> 0x1E0)
    
    // ... D'autres registres DSP seraient nécessaires pour un réglage fin du JPEG
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

void CAM::CaptureFrame(const char* filename) {
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("Erreur : Impossible d'ouvrir le fichier SD");
        return;
    }

    // 1. Attendre la fin du VSYNC actuel (s'il y en a un)
    while (readGPIO(VSYNC_GPIO) == 1);
    // 2. Attendre le début d'une nouvelle image (VSYNC monte)
    while (readGPIO(VSYNC_GPIO) == 0);
    // 3. Attendre que VSYNC redescende (début effectif des données)
    while (readGPIO(VSYNC_GPIO) == 1);

    Serial.println("Capture en cours...");

    // Boucle de capture tant que VSYNC est bas (pendant l'image)
    // Note : Pour du JPEG, on cherche les marqueurs SOI (0xFFD8) et EOI (0xFFD9)
    while (readGPIO(VSYNC_GPIO) == 0) {
        // Attendre que HREF soit haut (ligne active)
        if (readGPIO(HREF_GPIO) == 1) {
            // Attendre le front montant de PCLK pour lire l'octet
            while (readGPIO(PCLK_GPIO) == 0); 
            
            uint8_t data = readByteFast();
            file.write(data);

            // Attendre que PCLK redescende
            while (readGPIO(PCLK_GPIO) == 1);
        }
    }

    file.close();
    Serial.println("Capture terminée.");
}

#endif