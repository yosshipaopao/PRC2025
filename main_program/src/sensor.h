#ifndef _sensor_h
#define _sensor_h

#include <Arduino.h>

class Sensor
{
public:
    Sensor(int *pins, int pin_size, int threshold);
    void setup();
    void read();
    bool isBlack(int pin);
    bool get(int pin);
    int val(int pin);
    int state();
    void print_raw();

private:
    int *pins;
    int pin_size;
    int values[100];
    int threshold;
};

Sensor::Sensor(int *pins, int pin_size, int threshold)
{
    this->pins = pins;
    this->pin_size = pin_size;
    this->threshold = threshold;
}

void Sensor::setup()
{
    for (int i = 0; i < pin_size; i++)
    {
        pinMode(pins[i], INPUT);
    }
}

void Sensor::read()
{
    for (int i = 0; i < pin_size; i++)
    {
        values[i] = analogRead(pins[i]);
    }
}

bool Sensor::isBlack(int pin)
{
    return analogRead(pin) > threshold;
}

bool Sensor::get(int pin)
{
    return values[pin] > threshold;
}

int Sensor::val(int pin)
{
    return values[pin];
}

int Sensor::state()
{
    int state = 0;
    for (int i = 0; i < pin_size; i++)
    {
        if (isBlack(pins[i]))
        {
            state |= 1 << i;
        }
    }
    return state;
}

void Sensor::print_raw()
{
    for (int i = 0; i < pin_size; i++)
    {
        Serial.print(values[i]);
        Serial.print(" ");
    }
    Serial.println();
}

#endif