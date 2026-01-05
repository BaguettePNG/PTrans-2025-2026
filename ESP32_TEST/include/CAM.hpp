#ifndef __CAM_HPP__
#define __CAM_HPP__

#include <Arduino.h>
#include <Wire.h>
#include <SD.h>

#define OV2640_ADDR 0x30

// --- Pins ---
#define SIOD_GPIO_NUM     21
#define SIOC_GPIO_NUM     22
#define D0_GPIO     39
#define D1_GPIO     35
#define D2_GPIO     34
#define D3_GPIO     32
#define D4_GPIO     27
#define D5_GPIO     23
#define D6_GPIO     19
#define D7_GPIO     18

#define VSYNC_GPIO  4 
#define HREF_GPIO   25 
#define PCLK_GPIO   5 
#define XCLK_GPIO   -1

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

        // Configuration de l'horloge externe XCLK à 24 MHz
        pinMode(XCLK_GPIO, OUTPUT);
        ledcSetup(0, 24000000, 1); // Canal 0, 24 MHz, résolution 1 bit
        ledcAttachPin(XCLK_GPIO, 0);
        ledcWrite(0, 1); // Devoir 50%

        return RegisterInit();
    }

    void CaptureFrame(const char* filename);
    void CaptureFrameBMP(const char* filename);

private :
    void writeReg(uint8_t reg, uint8_t val) {
        Wire.beginTransmission(OV2640_ADDR);
        Wire.write(reg);
        Wire.write(val);
        Wire.endTransmission();
        delay(5); 
    }

    bool RegisterInit()
    {
        writeReg(0xFF, 0x01);  
        writeReg(0x12, 0x80); // reset
        delay(200);

        // --- Horloges ---
        writeReg(0xFF, 0x00);
        writeReg(0x2C, 0xFF);
        writeReg(0x2E, 0xDF);

        // --- PCLK très lent ---
        writeReg(0xFF, 0x01);
        writeReg(0x11, 0x00);   // ÷32 (ESSENTIEL)

        // --- VSYNC normal ---
        writeReg(0x15, 0x00);

        // --- RGB565 ---
        writeReg(0xFF, 0x00);
        writeReg(0xDA, 0x01);   // enable output
        writeReg(0xD7, 0x03);   // RGB
        writeReg(0xDF, 0x02);   // RGB565

        /*
        // --- Configuration RGB ---
        writeReg(0xFF, 0x01);
        writeReg(0x12, 0x80); // Reset
        writeReg(0xFF, 0x01);
        writeReg(0x11, 0x01); // Prescaler
        writeReg(0x12, 0x04); // COM7: Sortie RGB
        writeReg(0x8c, 0x00); // Pas de RGB444
        writeReg(0x40, 0xd0); // COM15: RGB565
        writeReg(0xFF, 0x00);
        writeReg(0xDA, 0x01); // Normal mode
        writeReg(0xFF, 0x01);
        writeReg(0x12, 0x80); // Reset
        writeReg(0xFF, 0x01);
        writeReg(0x11, 0x01); // Prescaler
        writeReg(0x12, 0x84); // COM7: Sortie RGB
        writeReg(0xFF, 0x01);
        writeReg(0x42, 0xC3); // COM9: RGB565
        writeReg(0xFF, 0x82);
        writeReg(0xDA, 0x83); // Normal mode
        */
        return true;
    }


    inline uint8_t readByteFast() {
        uint32_t r0 = GPIO.in;
        uint32_t r1 = GPIO.in1.data;
        uint8_t val = 0;
        val |= ((r1 >> (D0_GPIO - 32)) & 1) << 0;  // D0 GPIO39
        val |= ((r1 >> (D1_GPIO - 32)) & 1) << 1;  // D1 GPIO35
        val |= ((r1 >> (D2_GPIO - 32)) & 1) << 2;  // D2 GPIO34
        val |= ((r1 >> (D3_GPIO - 32)) & 1) << 3;  // D3 GPIO32
        val |= ((r0 >> D4_GPIO) & 1) << 4;         // D4 GPIO27
        val |= ((r0 >> D5_GPIO) & 1) << 5;         // D5 GPIO23
        val |= ((r0 >> D6_GPIO) & 1) << 6;         // D6 GPIO19
        val |= ((r0 >> D7_GPIO) & 1) << 7;         // D7 GPIO18
        return val;
    }

    void writeBMPHeader(File &file, int width, int height) 
    {
        uint32_t fileSize = 54 + (width * height * 2); // 2 octets par pixel (RGB565)
        uint32_t offset = 54;
        uint32_t headerSize = 40;
        uint16_t planes = 1;
        uint16_t bitCount = 16; // RGB565
        uint32_t compression = 3; // BI_BITFIELDS pour RGB565
        uint32_t sizeImage = width * height * 2;
        uint8_t zero[4] = {0,0,0,0};

        file.write('B'); file.write('M');           // Signature
        file.write((uint8_t*)&fileSize, 4);        // Taille fichier
        file.write(zero, 4);                       // Réservé
        file.write((uint8_t*)&offset, 4);          // Offset pixels

        file.write((uint8_t*)&headerSize, 4);      // Taille en-tête DIB
        file.write((uint8_t*)&width, 4);           // Largeur
        file.write((uint8_t*)&height, 4);          // Hauteur
        file.write((uint8_t*)&planes, 2);          // Plans
        file.write((uint8_t*)&bitCount, 2);        // Bits par pixel
        file.write((uint8_t*)&compression, 4);     // Compression
        file.write((uint8_t*)&sizeImage, 4);       // Taille image
        file.write(zero, 4);                       // Xppm
        file.write(zero, 4);                       // Yppm
        file.write(zero, 4);                       // Couleurs totales
        file.write(zero, 4);                       // Couleurs importantes

        // Masques de couleur pour RGB565 (Indispensable pour la lecture Windows/Linux)
        uint32_t rMask = 0xF800;
        uint32_t gMask = 0x07E0;
        uint32_t bMask = 0x001F;
        file.write((uint8_t*)&rMask, 4);
        file.write((uint8_t*)&gMask, 4);
        file.write((uint8_t*)&bMask, 4);
    }


    for (uint32_t y = 0; y < h; y++) 
    {

        // HREF
        while (!(*gpio & (1 << HREF_GPIO)));

        uint8_t* p = line;

        for (uint32_t x = 0; x < w; x++) {

            while (!(*gpio & (1 << PCLK_GPIO)));
            *p++ = readByteFast();
            while ( (*gpio & (1 << PCLK_GPIO)));

            while (!(*gpio & (1 << PCLK_GPIO)));
            *p++ = readByteFast();
            while ( (*gpio & (1 << PCLK_GPIO)));
        }

        file.write(line, w * 2);
    }

    file.close();
    Serial.println("Capture OK");
}


void CAM::CaptureFrameBMP(const char* path) {
    setCpuFrequencyMhz(240);

    int width = 320;
    int height = 240;

    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Erreur SD");
        return;
    }

    writeBMPHeader(file, width, height);

    static uint8_t line[width * 2];
    volatile uint32_t* gpio = &GPIO.in;

    // VSYNC
    while (!(*gpio & (1 << VSYNC_GPIO)));
    while ( (*gpio & (1 << VSYNC_GPIO)));

    for (int y = 0; y < height; y++) {
        // HREF
        while (!(*gpio & (1 << HREF_GPIO)));

        uint8_t* p = line;
        for (int x = 0; x < width; x++) {
            while (!(*gpio & (1 << PCLK_GPIO)));
            *p++ = readByteFast();
            while ( (*gpio & (1 << PCLK_GPIO)));

            while (!(*gpio & (1 << PCLK_GPIO)));
            *p++ = readByteFast();
            while ( (*gpio & (1 << PCLK_GPIO)));
        }

        file.write(line, width * 2);
    }

    file.close();
    Serial.println("Image sauvegardée !");
}



#endif