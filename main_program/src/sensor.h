#ifndef _sensor_h
#define _sensor_h

#include <Arduino.h>
#include "debug.h"

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
    void print_state();

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
        Debug::print(values[i]);
        Debug::print(" ");
    }
    Debug::println();
}

void Sensor::print_state()
{
    int current_state = state();
    Debug::print("0b");
    for (int i = pin_size - 1; i >= 0; i--)
    {
        Debug::print((current_state & (1 << i)) ? "1" : "0");
    }
    Debug::println();
}

#endif