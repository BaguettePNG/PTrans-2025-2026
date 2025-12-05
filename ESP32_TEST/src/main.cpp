
#include <Arduino.h>

#include "GPS.hpp"
#include "RTC.hpp"
#include "LDR.hpp"
#include "BAT.hpp"
#include "DHT.hpp"

// Etat de l'automate
enum State
{
  REPOS,
  POWER_UP,
  LUMINOSITY_MEASURE,
  LUMINOSITY_IR,
  PICTURE,
  TEMP,
  HUMIDITY,
  HUMIDITY_TEMP,
  GEOLOCATION,
  DATE_TIME,
  BATTERY,
  POWER_DOWN
};
State state;

void setup()
{
  Serial.begin(115200);

  state = REPOS;
}

void loop()
{
  switch (state)
  {
    case REPOS:
      
      break;
    
    default:
      break;
  }
}