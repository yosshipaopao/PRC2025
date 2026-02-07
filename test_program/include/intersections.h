#pragma once

class Sensors;
class Motors;

class Intersections
{
public:
    enum DetectionType
    {
        NONE,
        CROSS,
        LEFT_T,
        ALL_ACTIVE
    };

private:
    Sensors &sensors;
    Motors &motors;
    bool crossDetected;
    bool leftTDetected;
    bool allActiveDetected;
    bool allActiveTurnLeft;
    int leftTCount;
    int allActiveCount;
    unsigned long allActiveDetectedMs;
    // CROSS検知用: 各センサー検知時刻
    unsigned long crossSensorFlags[6];

public:
    Intersections(Sensors &s, Motors &m);
    void updateFlags();
    DetectionType consumeDetection();
    void turnLeftUntilLine(float &lastError);
};
