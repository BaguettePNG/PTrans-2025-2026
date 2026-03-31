#include <Arduino.h>
#include "SDM.h"
#include "CAM.hpp"
#include <DHT.h>
#include "RTC.hpp"
#include "GPS.hpp"
#include "BAT.hpp"
#include "SENDWIFI.hpp"

#include <Adafruit_NeoPixel.h>
#define LED_PIN 48 // ta pin NeoPixel
#define NUM_LEDS 1 // 1 LED RGB intégrée
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

BAT bat(BAT_PIN);
RTC rtc;
GPS gps(GPS_RX_PIN, GPS_TX_PIN);
CAM cam;
DHT dht(DHT_PIN, DHT22);
SendData send;

RTC_DATA_ATTR bool activate = false;
RTC_DATA_ATTR int lastRefreshDay;

#define WIDTH 1600
#define HEIGHT 1200


void setup()
{
    Serial.begin(115200);

    pinMode(PWR_U_D, OUTPUT);
    pinMode(GPS_PWR_UD, OUTPUT);
    pinMode(LED_DEB, OUTPUT);

    digitalWrite(PWR_U_D, HIGH);   // Alimentation ON
    digitalWrite(GPS_PWR_UD, LOW); // Alimentation GPS OFF

    delay(1000); // Attente stabilisation

    initSD();

    if (!cam.begin()) {
        Serial.println("Initialisation caméra échouée, arrêt setup.");
        return;
    }

    cam.takeAndSavePhoto("/test_1.jpg");
    delay(1000);
    cam.takeAndSavePhoto("/test_2.jpg");

    dht.begin();
    
    pinMode(LED_DEB, OUTPUT);
}

void loop()
{
    digitalWrite(LED_DEB, HIGH);
    delay(1000);
    digitalWrite(LED_DEB, LOW);
    delay(1000);

    Serial.println("Lecture capteurs...");
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    Serial.println("Température: " + String(temperature) + " °C");
    Serial.println("Humidité: " + String(humidity) + " %");


}