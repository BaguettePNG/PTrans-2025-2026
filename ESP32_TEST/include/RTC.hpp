#ifndef __RTC_HPP__
#define __RTC_HPP__

#include <Arduino.h>
#include <time.h>
#include <sys/time.h>

class RTC
{
public:
    RTC() {}  // Rien à configurer pour GPS

    String getDateTime()
    {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) return String("Erreur temps");

        char buffer[25];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return String(buffer);
    }

    void updateFromGPS(String hours, String minutes, String seconds, String day, String month, String year)
    {
        if (hours == "" || day == "" || month == "" || year == "") return;

        struct tm timeinfo;
        memset(&timeinfo, 0, sizeof(struct tm));

        timeinfo.tm_hour = hours.toInt();
        timeinfo.tm_min  = minutes.toInt();
        timeinfo.tm_sec  = seconds.toInt();

        timeinfo.tm_mday = day.toInt();
        timeinfo.tm_mon  = month.toInt() - 1;  // tm_mon = 0–11
        timeinfo.tm_year = year.toInt() - 1900; // année complète

        time_t t = mktime(&timeinfo);

        struct timeval now = { .tv_sec = t, .tv_usec = 0 };
        settimeofday(&now, nullptr);
    }
};

#endif
