#include <Arduino.h>
#include "config.h"
#include "debug.h"
#include "pattern.h"

Pattern::Pattern(int p10, int p11, int p12, int confirm)
    : pin10(p10), pin11(p11), pin12(p12), confirmCount(confirm),
      currentPattern(0), lastPattern(0), patternCount(0), detectedPattern(0)
{
}

int Pattern::readPins()
{
    int p10 = digitalRead(pin10) == LOW ? 1 : 0;
    int p11 = digitalRead(pin11) == LOW ? 1 : 0;
    int p12 = digitalRead(pin12) == LOW ? 1 : 0;

    int pattern = (p10 << 2) | (p11 << 1) | p12;
    return pattern;
}

void Pattern::setup()
{
    pinMode(pin10, INPUT_PULLUP);
    pinMode(pin11, INPUT_PULLUP);
    pinMode(pin12, INPUT_PULLUP);
}

void Pattern::updateDetection()
{
    currentPattern = readPins();

    if (currentPattern == 0)
    {
        patternCount = 0;
        lastPattern = 0;
        return;
    }

    if (currentPattern == lastPattern)
    {
        patternCount++;

        if (patternCount >= confirmCount)
        {
            if (detectedPattern != currentPattern)
            {
                detectedPattern = currentPattern;
                Debug::print("PATTERN_DETECTED: ");
                Debug::println(detectedPattern);
            }
        }
    }
    else
    {
        lastPattern = currentPattern;
        patternCount = 1;
    }
}

bool Pattern::hasDetected()
{
    return detectedPattern > 0;
}

int Pattern::consumeDetected()
{
    int value = detectedPattern;
    detectedPattern = 0;
    patternCount = 0;
    return value;
}
