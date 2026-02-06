#pragma once

class Sensors;
class Motors;

class Intersections
{
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
    unsigned long lastAllOnMs;
    bool allOnSeen;

public:
    Intersections(Sensors &s, Motors &m);
    void updateFlags();
    bool consumeCrossDetected();
    bool consumeLeftTDetected();
    bool consumeAllActiveTurnLeft();
    void turnLeftUntilLine(float &lastError);
};
