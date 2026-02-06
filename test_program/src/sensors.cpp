#include <Arduino.h>
#include <math.h>
#include "config.h"
#include "sensors.h"

Sensors::Sensors(const int *pins, int count, int offset, int activeCount)
    : sensorPins(pins), sensorCount(count), activeSensorCount(activeCount), activeOffset(offset)
{
}

void Sensors::setup()
{
    for (int i = 0; i < sensorCount; i++)
    {
        pinMode(sensorPins[i], INPUT);
    }
}

int Sensors::getSensorCount() const
{
    return sensorCount;
}

int Sensors::getActiveSensorCount() const
{
    return activeSensorCount;
}

int Sensors::readRaw(int index)
{
    if (index < 0 || index >= sensorCount)
    {
        return 0;
    }
    return analogRead(sensorPins[index]);
}

int Sensors::readActiveRaw(int index)
{
    if (index < 0 || index >= activeSensorCount)
    {
        return 0;
    }
    int pinIndex = activeOffset + index;
    if (pinIndex < 0 || pinIndex >= sensorCount)
    {
        return 0;
    }
    return analogRead(sensorPins[pinIndex]);
}

bool Sensors::isLineDetectedByIndex(int index)
{
    return readRaw(index) > Config::LINE_SENSOR_THRESHOLD; // 黒線が高値前提
}

bool Sensors::isActiveLineDetected(int index)
{
    return readActiveRaw(index) > Config::LINE_SENSOR_THRESHOLD; // 黒線が高値前提
}

float Sensors::readLineError(float lastError)
{
    const int weights[Config::LINE_ACTIVE_SENSOR_COUNT] = {-2, -1, 0, 1, 2};
    long weightedSum = 0;
    long total = 0;

    for (int i = 0; i < activeSensorCount; i++)
    {
        int raw = readActiveRaw(i);
        int value = raw - Config::LINE_SENSOR_THRESHOLD; // 黒線が高値前提
        if (value < 0)
        {
            value = 0;
        }
        weightedSum += (long)weights[i] * value;
        total += value;
    }

    if (total == 0)
    {
        if (fabs(lastError) < Config::LINE_ERROR_DEADBAND)
        {
            return 0.0f; // 直進維持
        }
        if (lastError > 0)
        {
            return 3.0f; // 右側に強く曲がる
        }
        return -3.0f; // 左側に強く曲がる
    }

    return (float)weightedSum / (float)total;
}
