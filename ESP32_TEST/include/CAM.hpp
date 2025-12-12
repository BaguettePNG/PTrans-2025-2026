#ifndef __CAM_HPP__
#define __CAM_HPP__

#include <Arduino.h>
#include <Wire.h>
#include <SD.h>

// GPIO OV7670 
#define XCLK_GPIO_NUM    23 
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y2_GPIO_NUM      4   // D0
#define Y3_GPIO_NUM      5   // D1
#define Y4_GPIO_NUM      18  // D2
#define Y5_GPIO_NUM      19  // D3
#define Y6_GPIO_NUM      36  // D4
#define Y7_GPIO_NUM      39  // D5
#define Y8_GPIO_NUM      34  // D6
#define Y9_GPIO_NUM      35  // D7

#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    21
#define PCLK_GPIO_NUM    22

// Résolutions
#define WIDTH_QQVGA  160
#define HEIGHT_QQVGA 120

class CAM
{
private:
    int width  = WIDTH_QQVGA;
    int height = HEIGHT_QQVGA;

    const unsigned long TIMEOUT_MS = 1000;

public:
    CAM() {}

    bool begin();
    bool captureToSD(const char* filename);

private:
    void writeReg(uint8_t reg, uint8_t val);
    void init_OV7670();
    uint8_t readByte();
    bool waitForPin(int pin, int state, unsigned long timeout);
};

// -----------------------------------------------------------------------------
// INIT
// -----------------------------------------------------------------------------
bool CAM::begin()
{
    // XCLK 8 MHz
    ledcSetup(0, 8000000, 1);
    ledcAttachPin(XCLK_GPIO_NUM, 0);
    ledcWrite(0, 1);
    delay(30);

    pinMode(PCLK_GPIO_NUM,  INPUT);
    pinMode(VSYNC_GPIO_NUM, INPUT);
    pinMode(HREF_GPIO_NUM,  INPUT);

    pinMode(Y2_GPIO_NUM, INPUT);
    pinMode(Y3_GPIO_NUM, INPUT);
    pinMode(Y4_GPIO_NUM, INPUT);
    pinMode(Y5_GPIO_NUM, INPUT);
    pinMode(Y6_GPIO_NUM, INPUT);
    pinMode(Y7_GPIO_NUM, INPUT);
    pinMode(Y8_GPIO_NUM, INPUT);
    pinMode(Y9_GPIO_NUM, INPUT);

    Wire.begin();
    delay(100);

    init_OV7670();
    return true;
}

// -----------------------------------------------------------------------------
// OV7670 WRITE REG
// -----------------------------------------------------------------------------
void CAM::writeReg(uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(0x21);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
    delay(2);
}

// -----------------------------------------------------------------------------
// OV7670 CONFIG QQVGA RGB565 (CORRECTE)
// -----------------------------------------------------------------------------
void CAM::init_OV7670()
{
    writeReg(0x12, 0x80);
    delay(100);

    writeReg(0x12, 0x14); // RGB output + QQVGA
    writeReg(0x40, 0x10); // RGB565

    writeReg(0x11, 0x01); // Prescaler
    writeReg(0x6B, 0x4A); // PLL

    // Windowing QQVGA
    writeReg(0x17, 0x3F); // HSTART
    writeReg(0x18, 0x03); // HSTOP
    writeReg(0x32, 0xA4); // HREF

    writeReg(0x19, 0x02); // VSTART
    writeReg(0x1A, 0x7A); // VSTOP

    writeReg(0x15, 0x00); // normal polarities
}

// -----------------------------------------------------------------------------
// BYTE READ (CORRECTION : REMAP BIT → D0..D7)
// -----------------------------------------------------------------------------
uint8_t CAM::readByte()
{
    return 
      (digitalRead(Y2_GPIO_NUM) << 0) |
      (digitalRead(Y3_GPIO_NUM) << 1) |
      (digitalRead(Y4_GPIO_NUM) << 2) |
      (digitalRead(Y5_GPIO_NUM) << 3) |
      (digitalRead(Y6_GPIO_NUM) << 4) |
      (digitalRead(Y7_GPIO_NUM) << 5) |
      (digitalRead(Y8_GPIO_NUM) << 6) |
      (digitalRead(Y9_GPIO_NUM) << 7);
}

// -----------------------------------------------------------------------------
// WAIT PIN
// -----------------------------------------------------------------------------
bool CAM::waitForPin(int pin, int state, unsigned long timeout)
{
    unsigned long start = millis();
    while (digitalRead(pin) != state)
    {
        if (millis() - start > timeout)
            return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// CAPTURE
// -----------------------------------------------------------------------------
bool CAM::captureToSD(const char* filename)
{
    File f = SD.open(filename, FILE_WRITE);
    if (!f) return false;

    Serial.println("Attente VSYNC...");

    if (!waitForPin(VSYNC_GPIO_NUM, LOW, TIMEOUT_MS)) { Serial.println("VSYNC LOW timeout"); return false; }
    if (!waitForPin(VSYNC_GPIO_NUM, HIGH, TIMEOUT_MS)) { Serial.println("VSYNC HIGH timeout"); return false; }

    Serial.println("Capture...");

    for (int y = 0; y < height; y++)
    {
        if (!waitForPin(HREF_GPIO_NUM, HIGH, TIMEOUT_MS)) { Serial.printf("HREF start timeout ligne %d\n", y); return false; }

        for (int x = 0; x < width; x++)
        {
            if (!waitForPin(PCLK_GPIO_NUM, HIGH, TIMEOUT_MS)) return false;
            uint8_t msb = readByte();
            if (!waitForPin(PCLK_GPIO_NUM, LOW, TIMEOUT_MS)) return false;

            if (!waitForPin(PCLK_GPIO_NUM, HIGH, TIMEOUT_MS)) return false;
            uint8_t lsb = readByte();
            if (!waitForPin(PCLK_GPIO_NUM, LOW, TIMEOUT_MS)) return false;

            f.write(msb);
            f.write(lsb);
        }

        if (!waitForPin(HREF_GPIO_NUM, LOW, TIMEOUT_MS)) return false;
    }

    f.close();
    Serial.println("Capture terminée !");
    return true;
}

#endif
