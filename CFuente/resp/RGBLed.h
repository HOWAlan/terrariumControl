#ifndef RGBLED_H
#define RGBLED_H

#include <Arduino.h>
#include <time.h>
#define OK_STATUS 0
#define ERROR_STATUS 1
#define PROCESS_STATUS 2
#define RED 0
#define BLUE 1
#define GREEN 2

enum LogicType {
    ACTIVE_LOW = LOW,
    ACTIVE_HIGH = HIGH
};
class RGBLed{
    private:
    uint8_t pinR;
    uint8_t pinG;
    uint8_t pinB;
    LogicType logic;

    public:
    RGBLed(uint8_t pinR,uint8_t pinG,uint8_t pinB, LogicType logic = ACTIVE_HIGH);
    void On(uint8_t color);
    void On(uint8_t r,uint8_t g, uint8_t b);
    void Off();
    void ShowStatus(uint8_t status);

};
#endif