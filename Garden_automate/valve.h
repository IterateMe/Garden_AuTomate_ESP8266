#include <Arduino.h>

class Valve{
    private:
        bool state = 1;
        uint8_t pin;
    public:
        Valve(uint8_t pinValve);
        void switchOnOff();
        bool getState();
        void turnOn();
        void turnOff();
};
