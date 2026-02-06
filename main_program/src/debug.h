#ifndef _debug_h
#define _debug_h

#include <Arduino.h>

// コンパイル時設定: DEBUG_ENABLED を 1 で有効、0 で無効
#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED 0
#endif

class Debug
{
public:
    static void begin(long baud);

    template <typename T>
    static void print(T value)
    {
#if DEBUG_ENABLED
        Serial.print(value);
#endif
    }

    template <typename T>
    static void println(T value)
    {
#if DEBUG_ENABLED
        Serial.println(value);
#endif
    }

    static void println()
    {
#if DEBUG_ENABLED
        Serial.println();
#endif
    }
};

void Debug::begin(long baud)
{
#if DEBUG_ENABLED
    Serial.begin(baud);
#endif
}

#endif
