#include <Arduino.h>

#define XCLK 10
#define PCLK 11
#define SDA 23
#define SCL 24
#define VREF 25
#define HREF 26
#define PWD 27
#define RST 28
#define D0 0
#define D1 1
#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7

void setup() 
{
  // Moniteur Serie
  Serial.begin(115200);
  Serial.print("\n");

  // Pinout
  pinMode(XCLK, OUTPUT);
  pinMode(PCLK, OUTPUT);
  pinMode(PWD, OUTPUT);
  pinMode(RST, OUTPUT);

  pinMode(HREF, INPUT);
  pinMode(VREF, INPUT);
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);
}



void loop()
{

}