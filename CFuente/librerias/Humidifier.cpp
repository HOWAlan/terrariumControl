#include "Humidifier.h"
Humidifier::Humidifier(uint8_t pin, LogicType _logic){
    pinOut = pin;
    logic = _logic;
    pinMode(pinOut,OUTPUT);
    digitalWrite(pinOut,!logic);
}

void Humidifier::Shot(uint8_t ticks, uint delayTime)
{
    for(int i = 0; i<ticks; i++)
    {
            digitalWrite(pinOut,logic);
            delay(delayTime);
            digitalWrite(pinOut,!logic);
            delay(delayTime);
    }
    digitalWrite(pinOut,!logic);
}