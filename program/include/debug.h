#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

// デバッグ機能のオン/オフ（0=無効, 1=有効）
#define DEBUG_ENABLED 0

/**
 * デバッグ出力クラス
 * 条件付きコンパイルでデバッグ出力を制御
 */
class Debug
{
public:
    /**
     * シリアル通信初期化
     * @param baud ボーレート
     */
    static void begin(long baud)
    {
#if DEBUG_ENABLED
        Serial.begin(baud);
#endif
    }

    /**
     * デバッグメッセージを出力
     * @param value 出力する値
     */
    template <typename T>
    static void print(T value)
    {
#if DEBUG_ENABLED
        Serial.print(value);
#endif
    }

    /**
     * デバッグメッセージを出力（改行付き）
     * @param value 出力する値
     */
    template <typename T>
    static void println(T value)
    {
#if DEBUG_ENABLED
        Serial.println(value);
#endif
    }

    /**
     * 改行のみ出力
     */
    static void println()
    {
#if DEBUG_ENABLED
        Serial.println();
#endif
    }
};

#endif // DEBUG_H
