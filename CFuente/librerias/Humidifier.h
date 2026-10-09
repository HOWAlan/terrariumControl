#ifndef HUMIDIFIER_H
#define HUMIDIFIER_H

#include <Arduino.h>
#include <time.h>
// Definición de tipos de actuador equivalente a Constants.py
enum LogicType {
    ACTIVE_LOW = LOW,
    ACTIVE_HIGH = HIGH
};
class Humidifier{
    private:
    uint8_t pinOut;
    LogicType logic;

    public:
    Humidifier(uint8_t pin, LogicType logic = ACTIVE_HIGH);
    void Shot(uint8_t ticks, uint delayTime);
};
#endif