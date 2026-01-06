#include <Arduino.h>
#include "SDM.h"
#include "CAM.hpp"
#include <DHT.h>
#include "RTC.hpp"
#include "GPS.hpp"

RTC rtc;
GPS gps(GPS_RX_PIN, GPS_TX_PIN);
CAM cam;
DHT dht(DHT_PIN, DHT22);

void setup() {
    Serial.begin(115200);
    while(!Serial); // Attend l'ouverture du moniteur
    delay(2000); 

    Serial.println("Initialisation de la carte SD...");
    initSD();

    Serial.println("Initialisation de la caméra...");
    cam.begin();

    Serial.println("Initialisation du capteur DHT22...");
    dht.begin();

    Serial.println("SETUP TERMINÉ");
}

void loop() 
{
    if (Serial.available() && Serial.read() == 'c') {
        
        // Lecture des capteurs
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        // Vérification si la lecture a réussi
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Erreur de lecture DHT22 !");
            return; 
        }

        Serial.printf("Temp: %.2fC | Hum: %.2f%%\n", temperature, humidity);

        // Nettoyage du nom de fichier (on remplace les points par des tirets pour éviter les bugs SD)
        // Format : /pic_28_25_50.jpg (plus sûr)
        String fileName = "/pic_" + String((int)(temperature)) + 
                          "_" + String((int)humidity) + ".jpg";

        Serial.print("Sauvegarde sous : ");
        Serial.println(fileName);

        cam.takeAndSavePhoto(fileName.c_str());
    }
}