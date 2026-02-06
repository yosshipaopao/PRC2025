#pragma once

class Pattern
{
private:
    int pin10, pin11, pin12;
    int confirmCount;
    int currentPattern;
    int lastPattern;
    int patternCount;
    int detectedPattern;

    int readPins();

public:
    Pattern(int p10, int p11, int p12, int confirm);
    void setup();
    void updateDetection();
    bool hasDetected();
    int consumeDetected();
    void execute(int pattern);
};
