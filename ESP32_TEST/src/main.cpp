#include <Arduino.h>

unsigned long start;
unsigned long stop;

void setup()
{
  Serial.begin(115200);         
}

void loop()
{
  start = micros();

  // Votre Codes

  stop = micros();
  unsigned long executionTime = stop - start;

  Serial.print(" | Temps d'exécution: ");
  Serial.print(executionTime);
  Serial.println(" μs");
  Serial.print(executionTime / 1000.0, 3);
  Serial.println(" ms");
}