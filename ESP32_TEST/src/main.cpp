
#include <Arduino.h>
#include "GPS.hpp"
#include "RTC.hpp"

GPS gps(16, 17); // RX, TX
RTC rtc;
void setup()
{
  Serial.begin(115200);
  while (!gps.gpsready())
  {
    Serial.println("En attente de données GPS...");
    delay(1000);
  }
  gps.update();
  rtc.updateFromGPS(gps.getHours(), gps.getMinutes(), gps.getSeconds(), gps.getDay(), gps.getMonth(), gps.getYear());

  Serial.println("Données GPS reçues et RTC mise à jour.");

  Serial.print("Latitude: ");
  Serial.println(gps.getLatitude());
  Serial.print("Longitude: ");
  Serial.println(gps.getLongitude());
  Serial.print("Heure UTC: ");
  Serial.print(gps.getHours());
  Serial.print(":");
  Serial.print(gps.getMinutes());
  Serial.print(":");
  Serial.println(gps.getSeconds());
  Serial.print("Date (JJ/MM/AAAA): ");
  Serial.print(gps.getDay());
  Serial.print("/");
  Serial.print(gps.getMonth());
  Serial.print("/");
  Serial.println(gps.getYear());

  Serial.println("===============================");
}
void loop()
{
  String dateTime = rtc.getDateTime();
  Serial.println("Date et Heure RTC : " + dateTime);

  delay(10000); // Attendre 10 secondes avant la prochaine mise à jour


}