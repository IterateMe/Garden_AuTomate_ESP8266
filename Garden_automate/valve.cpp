#include "valve.h"
#include <Arduino.h>

Valve::Valve(uint8_t pinValve)
{
    pin = pinValve;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, state);
}

void Valve::switchOnOff(){
    state = !state; 
    digitalWrite(pin, state);
}

bool Valve::getState(){
    return state;
}

void Valve::turnOn(){
    if(state){
        switchOnOff();
    }
}

void Valve::turnOff(){
    if(!state){
        switchOnOff();
    }
}
