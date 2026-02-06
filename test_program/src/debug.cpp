#include <Arduino.h>
#include "config.h"
#include "debug.h"
#include "sensors.h"

static unsigned long lastSensorPrintMs = 0;
static Sensors *globalSensors = nullptr;

// Debugクラスの実装
void Debug::begin(unsigned long baud)
{
#if ENABLE_SENSOR_PRINT
    Serial.begin(baud);
#else
    (void)baud;
#endif
}

void Debug::setSensors(Sensors *sensors)
{
    globalSensors = sensors;
}

void Debug::print(const char *str)
{
#if ENABLE_SENSOR_PRINT
    Serial.print(str);
#else
    (void)str;
#endif
}

void Debug::print(int value)
{
#if ENABLE_SENSOR_PRINT
    Serial.print(value);
#else
    (void)value;
#endif
}

void Debug::print(float value, int decimals)
{
#if ENABLE_SENSOR_PRINT
    Serial.print(value, decimals);
#else
    (void)value;
    (void)decimals;
#endif
}

void Debug::println(const char *str)
{
#if ENABLE_SENSOR_PRINT
    Serial.println(str);
#else
    (void)str;
#endif
}

void Debug::println(int value)
{
#if ENABLE_SENSOR_PRINT
    Serial.println(value);
#else
    (void)value;
#endif
}

void Debug::println(float value, int decimals)
{
#if ENABLE_SENSOR_PRINT
    Serial.println(value, decimals);
#else
    (void)value;
    (void)decimals;
#endif
}

void Debug::println()
{
#if ENABLE_SENSOR_PRINT
    Serial.println();
#endif
}

void Debug::sensorStates(float lastError)
{
    if (!globalSensors)
    {
        return;
    }

    unsigned long now = millis();
    if (now - lastSensorPrintMs < Config::DEBUG_SENSOR_PRINT_INTERVAL_MS)
    {
        return;
    }
    lastSensorPrintMs = now;

    int count = globalSensors->getSensorCount();
    for (int i = 0; i < count; i++)
    {
        int state = globalSensors->isLineDetectedByIndex(i) ? 1 : 0;
        Debug::print(state);
    }
    Debug::print(" E:");
    Debug::println(lastError, 2);
}
