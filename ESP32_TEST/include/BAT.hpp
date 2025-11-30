#ifndef __BAT_HPP__
#define __BAT_HPP__

#include <Arduino.h>
#include <stdio.h>

#define VOLTAGE 3.3
#define RESOLUTION 12
#define ADC_MAX 4095.0

class BAT
{
    private:
        unsigned int _pin;
        
    public:
        BAT(unsigned int pin);
        float ReadVoltage();
};

BAT::BAT(unsigned int pin)
{
    this->_pin = pin;

    pinMode(this->_pin, INPUT);
    analogReadResolution(RESOLUTION);
}

float BAT::ReadVoltage()
{
    int sensorValue = analogRead(this->_pin);
    float voltage = sensorValue * (VOLTAGE / ADC_MAX); 
    return voltage;
}

#endif