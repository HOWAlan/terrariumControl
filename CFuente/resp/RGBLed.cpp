#include "RGBLed.h"

RGBLed::RGBLed(uint8_t pR,uint8_t pG,uint8_t pB, LogicType _logic){
    pinR = pR;
    pinG = pG;
    pinB = pB;
    logic = _logic;

    pinMode(pinR,OUTPUT);
    pinMode(pinG,OUTPUT);
    pinMode(pinB,OUTPUT);

    digitalWrite(pinR,!logic);
    digitalWrite(pinG,!logic);
    digitalWrite(pinB,!logic);
}

void RGBLed::On(uint8_t color)
{
    switch(color)
    {
        case RED:
            On(255,0,0);
            break;
        case GREEN:
            On(0,255,0);
            break;
        case BLUE:
            On(0,0,255);
            break;
        default:
            Off();
            break;
    }
}
void RGBLed::On(uint8_t r,uint8_t g, uint8_t b)
{
    if (logic == ACTIVE_LOW) {
        r = 255 - r;
        g = 255 - g;
        b = 255 - b;
    }
    analogWrite(pinR,r);
    analogWrite(pinG,g);
    analogWrite(pinB,b);
}
void RGBLed::Off()
{
    On(0,0,0);
}
void RGBLed::ShowStatus(uint8_t status)
{
    switch(status)
    {
        case OK_STATUS:
            On(GREEN);
            break;
        case ERROR_STATUS:
            On(RED);
            break;
        case PROCESS_STATUS:
            for(int i = 0; i<2; i++)
            {
                On(255,255,0);
                delay(150);
                Off();
                delay(150);
            }
            break;
        default:
            Off();
            break; 
    }
}