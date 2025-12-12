#include <Arduino.h>

// Pin
#define DHT_PIN 4
#define BAT_PIN 34
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define LDR_PIN 35

#define LED_PIN 2
#define READ_WAKE_PIN 33 // PIR
#define DRIVE_WAKE_PIN 35 // PIR


// Include
#include <BAT.hpp>
#include <DHT.hpp>
#include <GPS.hpp>
#include <LDR.hpp>
#include <RTC.hpp>

// Instantiate objects
BAT bat(BAT_PIN); 
GPS gps(GPS_RX_PIN, GPS_TX_PIN); 
LDR ldr(LDR_PIN); 
RTC rtc;

// Automated functions
enum state
{
  REPOS,
  POWER_UP, READ_LUM, DRIVE_LUM, PICTURE, TEMP, HUM, 
};

state State = REPOS;

void main_collecteur() // Main
{
  while (digitalRead(READ_WAKE_PIN) != LOW)
  {
    switch (State)
    {
      case REPOS :
        State = POWER_UP;
        break;
  
      case POWER_UP :
        delay(1000); // Attente stabilisation capteurs
        State = READ_LUM;
        break;

      case READ_LUM :
        unsigned int lux = ldr.GetLuxValue();

        if (lux < 50)
        {
          State = PICTURE;
        }
        else
        {
          State = DRIVE_LUM;
        }
        break;

      case DRIVE_LUM :
        // Pilotage éclairage
        State = PICTURE;
        break;

      case PICTURE :
        // Prise de vue
        State = 

      default:
        break;
    }
  }
}

void mainTask(void *parameter) 
{
  setCpuFrequencyMhz(40);

  pinMode(DRIVE_WAKE_PIN, OUTPUT);
  pinMode(READ_WAKE_PIN, INPUT);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  if (cause == ESP_SLEEP_WAKEUP_EXT0) 
  {
    digitalWrite(DRIVE_WAKE_PIN, HIGH); // Force de PWR UP HIGH 

    // Exécute l'action UNE seule fois
    main_collecteur();
    
    while (digitalRead(READ_WAKE_PIN) == HIGH) {}
  }

  // Réarmer le front montant UNIQUEMENT quand la pin est basse
  esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_PIN, 1);
  esp_deep_sleep_start();
}

void setup() 
{
  // Tout mettre sur le CORE 0
  xTaskCreatePinnedToCore(mainTask, "mainTask", 4096, NULL, 1, NULL, 0);
}

void loop() {} // Pas Utilisé