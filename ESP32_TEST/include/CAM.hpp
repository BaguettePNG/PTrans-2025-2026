#ifndef __CAM_HPP__
#define __CAM_HPP__

#include <Arduino.h>
#include <Wire.h>
#include <SD.h>

#define OV2640_ADDR 0x30

// --- Pins ---
#define D0_GPIO     39
#define D1_GPIO     35
#define D2_GPIO     34
#define D3_GPIO     32
#define D4_GPIO     27
#define D5_GPIO     23
#define D6_GPIO     19
#define D7_GPIO     18

#define VSYNC_GPIO  4 
#define HREF_GPIO   2 
#define PCLK_GPIO   5 

class CAM {
public:
    CAM() {}

    bool begin() {
        pinMode(VSYNC_GPIO, INPUT);
        pinMode(HREF_GPIO, INPUT);
        pinMode(PCLK_GPIO, INPUT);
        pinMode(D0_GPIO, INPUT); pinMode(D1_GPIO, INPUT);
        pinMode(D2_GPIO, INPUT); pinMode(D3_GPIO, INPUT);
        pinMode(D4_GPIO, INPUT); pinMode(D5_GPIO, INPUT);
        pinMode(D6_GPIO, INPUT); pinMode(D7_GPIO, INPUT);

        return RegisterInit();
    }

    void CaptureFrameBMP(const char* filename);

private:
    void writeReg(uint8_t reg, uint8_t val) {
        Wire.beginTransmission(OV2640_ADDR);
        Wire.write(reg);
        Wire.write(val);
        Wire.endTransmission();
        delay(5); 
    }

bool RegisterInit() {
    // 1. Reset matériel (via registre)
    writeReg(0xFF, 0x01); // Bank Sensor
    writeReg(0x12, 0x80); // Reset
    delay(200);

    // 2. Initialisation des horloges (TRÈS IMPORTANT)
    writeReg(0xFF, 0x00); // Bank DSP
    writeReg(0x2C, 0xFF); // Active toutes les horloges (Power Down Off)
    writeReg(0x2E, 0xDF); // Active la logique système
    
    // 3. Configuration du Format de sortie
    // On va forcer le mode JPEG ou RGB mais surtout activer l'interface
    writeReg(0xFF, 0x00);
    writeReg(0xDA, 0x01); // Bit[0] : 1 = Output interface enable
    
    // 4. Inversion de polarité (pour voir si VSYNC passe à 0)
    writeReg(0xFF, 0x01); // Bank Sensor
    // COM10 (0x15) : Bit[1] contrôle la polarité de VSYNC
    // Si VSYNC est fixe à 1, essayez de mettre le bit 1 à 1 (0x02)
    writeReg(0x15, 0x02); 

    // 5. Réglage de l'horloge (PCLK)
    // On demande au capteur de diviser son horloge interne au max
    writeReg(0x11, 0x3F); 
    
    return true;
}

    // Lecture directe par accès registres (plus rapide que digitalRead)
    inline uint8_t readByteFast() {
        uint32_t r0 = GPIO.in;
        uint32_t r1 = GPIO.in1.data;
        uint8_t v = 0;
        if ((r1 >> (D0_GPIO - 32)) & 1) v |= 0x01;
        if ((r1 >> (D1_GPIO - 32)) & 1) v |= 0x02;
        if ((r1 >> (D2_GPIO - 32)) & 1) v |= 0x04;
        if ((r1 >> (D3_GPIO - 32)) & 1) v |= 0x08;
        if ((r0 >> D4_GPIO) & 1)        v |= 0x10;
        if ((r0 >> D5_GPIO) & 1)        v |= 0x20;
        if ((r0 >> D6_GPIO) & 1)        v |= 0x40;
        if ((r0 >> D7_GPIO) & 1)        v |= 0x80;
        return v;
    }
};

void CAM::CaptureFrameBMP(const char* filename) {
    File file = SD.open(filename, FILE_WRITE);
    if (!file) return;

    uint32_t w = 1600, h = 1200;
    uint32_t s = 54 + (w * h * 3);
    uint8_t head[54] = {0x42,0x4D,(uint8_t)s,(uint8_t)(s>>8),(uint8_t)(s>>16),(uint8_t)(s>>24),0,0,0,0,54,0,0,0,40,0,0,0,(uint8_t)w,(uint8_t)(w>>8),(uint8_t)(w>>16),(uint8_t)(w>>24),(uint8_t)h,(uint8_t)(h>>8),(uint8_t)(h>>16),(uint8_t)(h>>24),1,0,24,0};
    file.write(head, 54);

    uint8_t* buf = (uint8_t*)malloc(w * 3);
    if(!buf) return;

    // Cache des registres pour éviter de relire GPIO.in inutilement
    volatile uint32_t* gpio_in = (volatile uint32_t*)(GPIO_IN_REG);
    
    // Attente VSYNC (Début d'image)
    while (!((*gpio_in >> VSYNC_GPIO) & 1)); 
    while (((*gpio_in >> VSYNC_GPIO) & 1)); 

    for (int y = 0; y < h; y++) {
        // Attente HREF (Début de ligne)
        while (!((*gpio_in >> HREF_GPIO) & 1)); 
        
        uint8_t* p = buf;
        for (int x = 0; x < w; x++) {
            // Octet 1 : Attente PCLK High
            while (!((*gpio_in >> PCLK_GPIO) & 1));
            uint8_t b1 = readByteFast();
            while (((*gpio_in >> PCLK_GPIO) & 1)); // Attente PCLK Low

            // Octet 2 : Attente PCLK High
            while (!((*gpio_in >> PCLK_GPIO) & 1));
            uint8_t b2 = readByteFast();
            while (((*gpio_in >> PCLK_GPIO) & 1)); // Attente PCLK Low

            // Conversion ultra-rapide RGB565 -> BGR888
            *p++ = (b2 << 3);                   // B
            *p++ = ((b1 << 5) | (b2 >> 3)) & 0xFC; // G
            *p++ = (b1 & 0xF8);                 // R
        }
        
        // Écriture directe du buffer de ligne
        file.write(buf, w * 3);

        // On n'affiche le log que toutes les 50 lignes pour ne pas ralentir
        if (y % 50 == 0) {
            Serial.print("."); 
        }
    }

    free(buf);
    file.close();
    Serial.println("\nCapture terminée !");
}
#endif