#pragma once

#include "config.h"

#if ENABLE_SERVO
#include "ESP32Servo.h"

class ServoWrapper
{
private:
    Servo servo;
    int targetAngle;

    // タイマー制御用
    bool isTimedMode;
    unsigned long startTime;
    unsigned long duration;
    int secondAngle;

public:
    ServoWrapper();
    void setPeriodHertz(int hertz);
    void attach(int pin, int minPulseWidth, int maxPulseWidth);
    void write(int angle);
    void set(int angle1, int angle2, unsigned long durationMs);
    void applyWrite();
};
#else
class ServoWrapper
{
public:
    ServoWrapper() = default;
    void setPeriodHertz(int) {}
    void attach(int, int, int) {}
    void write(int) {}
    void set(int, int, unsigned long) {}
    void applyWrite() {}
};
#endif
