#ifndef __GPS_HPP__
#define __GPS_HPP__

#include <Arduino.h>
#include <sys/time.h>

class GPS
{
    private:
        int _RX;
        int _TX;

        String gpsData;

        String latitude;
        String longitude;

        String hours;
        String minutes;
        String seconds;

        String day;
        String month;
        String year;

    public:
        GPS(int RX, int TX);
        void update();
        bool gpsready();

        void Read();
        void ParseRMC(String trame);
        String convertToDecimal(String value, String dir);

        String getLatitude()  { return latitude; }
        String getLongitude() { return longitude; }
        String getHours()     { return hours; }
        String getMinutes()   { return minutes; }
        String getSeconds()   { return seconds; }
        String getDay()       { return day; }
        String getMonth()     { return month; }
        String getYear()      { return year; }  
};

GPS::GPS(int RX, int TX)
{
    this->_RX = RX;
    this->_TX = TX;

    Serial1.begin(9600, SERIAL_8N1, this->_RX, this->_TX);
}

bool GPS::gpsready()
{
    return Serial1.available();
}

void GPS::Read()
{    
    if (Serial1.available())
    {
        gpsData = Serial1.readStringUntil('\n');
    }
}

void GPS::update()
{
    do
    {
        Read();
    } while (!gpsData.startsWith("$GPRMC") && !gpsData.startsWith("$GNRMC"));
    
    ParseRMC(gpsData);
}

/*
=====================================================
 PARSE DE LA TRAME GPRMC
 Format :
 $GPRMC,hhmmss.sss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*CS
=====================================================
*/
void GPS::ParseRMC(String trame)
{
    int idx = 0;
    String fields[12];

    for (int i = 0; i < trame.length(); i++)
    {
        if (trame[i] == ',')
            idx++;
        else
            fields[idx] += trame[i];
    }

    if (fields[2] != "A")
    {
        Serial.println("GPS invalide !");
        return;
    }

    // Heure UTC (hhmmss)
    if (fields[1].length() >= 6)
    {
        hours   = fields[1].substring(0, 2);
        minutes = fields[1].substring(2, 4);
        seconds = fields[1].substring(4, 6);
    }

    // Latitude décimale
    latitude = convertToDecimal(fields[3], fields[4]);

    // Longitude décimale
    longitude = convertToDecimal(fields[5], fields[6]);

    // Date (ddmmyy)
    if (fields[9].length() == 6)
    {
        day   = fields[9].substring(0, 2);
        month = fields[9].substring(2, 4);
        year  = "20" + fields[9].substring(4, 6);
    }
}

/*
=====================================================
 CONVERSION DMS → DÉCIMAL
 Ex : 4807.038 N → 48.117300
=====================================================
*/
String GPS::convertToDecimal(String value, String dir)
{
    if (value.length() < 4) return "0";

    // Ex : 4807.038
    double val = value.toFloat();
    int deg = (int)(val / 100);
    double minutes = val - (deg * 100);
    double decimal = deg + (minutes / 60.0);

    if (dir == "S" || dir == "W") decimal *= -1;

    return String(decimal, 6);
}

#endif
