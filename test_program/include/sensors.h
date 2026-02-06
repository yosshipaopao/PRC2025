#pragma once

class Sensors
{
private:
    const int *sensorPins;
    int sensorCount;
    int activeSensorCount;
    int activeOffset;

public:
    Sensors(const int *pins, int count, int activeOffset, int activeCount);
    void setup();
    int getSensorCount() const;
    int getActiveSensorCount() const;
    int readRaw(int index);
    int readActiveRaw(int index);
    bool isLineDetectedByIndex(int index);
    bool isActiveLineDetected(int index);
    float readLineError(float lastError);
};
