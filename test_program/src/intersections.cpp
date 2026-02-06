#include <Arduino.h>
#include "config.h"
#include "debug.h"
#include "intersections.h"
#include "motors.h"
#include "sensors.h"

Intersections::Intersections(Sensors &s, Motors &m)
    : sensors(s), motors(m), crossDetected(false), leftTDetected(false),
      allActiveDetected(false), allActiveTurnLeft(false), leftTCount(0),
      allActiveCount(0), allActiveDetectedMs(0), lastAllOnMs(0), allOnSeen(false)
{
}

void Intersections::updateFlags()
{
    bool farLeft = sensors.isLineDetectedByIndex(Config::LINE_FAR_LEFT_INDEX);
    bool left = sensors.isActiveLineDetected(Config::LINE_LEFT_INDEX);
    bool midLeft = sensors.isActiveLineDetected(Config::LINE_MID_LEFT_INDEX);
    bool center = sensors.isActiveLineDetected(Config::LINE_CENTER_INDEX);
    bool midRight = sensors.isActiveLineDetected(Config::LINE_MID_RIGHT_INDEX);
    bool right = sensors.isActiveLineDetected(Config::LINE_RIGHT_INDEX);

    bool cross = farLeft && left && midLeft && center && midRight && right;
    bool leftT = farLeft && left && midLeft && center && !midRight && !right;
    bool allActive = left && midLeft && center && midRight && right;

    leftTCount = leftT ? (leftTCount + 1) : 0;
    allActiveCount = allActive ? (allActiveCount + 1) : 0;

    if (cross)
    {
        allOnSeen = true;
        lastAllOnMs = millis();
    }
    if (allOnSeen && (millis() - lastAllOnMs <= Config::CROSS_DETECT_WINDOW_MS))
    {
        if (!crossDetected)
        {
            crossDetected = true;
            Debug::println("CROSS_CONFIRMED");
        }
    }
    else if (allOnSeen && (millis() - lastAllOnMs > Config::CROSS_DETECT_WINDOW_MS))
    {
        allOnSeen = false;
    }

    leftTDetected = (!crossDetected) && (leftTCount >= Config::INTERSECTION_STABLE_COUNT);
    if (leftTDetected && leftTCount == Config::INTERSECTION_STABLE_COUNT)
    {
        Debug::println("LEFT_T_DETECTED");
    }

    if ((!crossDetected && !leftTDetected) && (allActiveCount >= Config::INTERSECTION_STABLE_COUNT))
    {
        if (!allActiveDetected)
        {
            allActiveDetected = true;
            allActiveDetectedMs = millis();
            Debug::println("ALL_ACTIVE_START");
        }
    }
    else if (allActiveDetected)
    {
        bool allOff = !farLeft && !left && !midLeft && !center && !midRight && !right;
        if (millis() - allActiveDetectedMs > 50)
        {
            allActiveDetected = false;
            allActiveCount = 0;
        }
        else if (allOff)
        {
            allActiveTurnLeft = true;
            allActiveDetected = false;
            allActiveCount = 0;
            Debug::println("ALL_ACTIVE_TURN_TRIGGER");
        }
    }

    if (crossDetected)
    {
        allOnSeen = false;
    }
}

bool Intersections::consumeCrossDetected()
{
    if (!crossDetected)
    {
        return false;
    }
    crossDetected = false;
    allOnSeen = false;
    return true;
}

bool Intersections::consumeLeftTDetected()
{
    if (!leftTDetected)
    {
        return false;
    }
    leftTDetected = false;
    leftTCount = 0;
    return true;
}

bool Intersections::consumeAllActiveTurnLeft()
{
    if (!allActiveTurnLeft)
    {
        return false;
    }
    allActiveTurnLeft = false;
    allActiveDetected = false;
    allActiveCount = 0;
    return true;
}

void Intersections::turnLeftUntilLine(float &lastError)
{
    motors.set(Config::LINE_BASE_SPEED, Config::LINE_BASE_SPEED);
    delay(Config::LEFT_TURN_OVERRUN_MS);

    motors.set(Config::LEFT_TURN_SPEED, -Config::LEFT_TURN_SPEED);
    while (sensors.isActiveLineDetected(Config::LINE_CENTER_INDEX))
    {
        motors.set(Config::LEFT_TURN_SPEED, -Config::LEFT_TURN_SPEED);
        Debug::sensorStates(lastError);
        delay(5);
    }

    delay(200); // 安定化待ち

    while (!sensors.isActiveLineDetected(Config::LINE_CENTER_INDEX))
    {
        motors.set(Config::LEFT_TURN_SPEED_SLOW, -Config::LEFT_TURN_SPEED_SLOW);
        Debug::sensorStates(lastError);
        delay(5);
    }

    lastError = 1.5f; // 左折後は左側に旋回する傾向を持たせる
}
