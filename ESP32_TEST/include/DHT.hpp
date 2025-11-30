#ifndef __DHT_HPP__
#define __DHT_HPP__

#include <Arduino.h>
#include <stdio.h>

class DHT
{
    private:
       unsigned int _pin;
    public:
        DHT(unsigned int pin);
};

DHT::DHT(unsigned int pin)
{
    this->_pin = pin;
}

#endif
