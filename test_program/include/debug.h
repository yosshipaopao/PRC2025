#pragma once

class Sensors;

class Debug
{
public:
    static void begin(unsigned long baud);
    static void setSensors(Sensors *sensors);
    static void sensorStates(float lastError);
    static void print(const char *str);
    static void print(int value);
    static void print(float value, int decimals = 2);
    static void println(const char *str);
    static void println(int value);
    static void println(float value, int decimals = 2);
    static void println();
};
