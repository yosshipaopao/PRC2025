#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

/**
 * 赤外線ラインセンサー管理クラス
 * 複数センサーの読み込みと黒線判定を実施
 */
class Sensor
{
private:
    int *pins;     // センサーピン配列
    int count;     // センサー個数
    int threshold; // 黒判定閾値

public:
    /**
     * コンストラクタ
     * @param pin_array センサーピン配列
     * @param pin_count センサー個数
     * @param thresh 黒判定閾値（デフォルト: 2000）
     */
    Sensor(int *pin_array, int pin_count, int thresh = 2000)
        : pins(nullptr), count(pin_count), threshold(thresh)
    {
        // 入力チェック
        if (pin_array == nullptr || pin_count <= 0)
        {
            count = 0;
            return;
        }

        pins = new int[count];
        for (int i = 0; i < count; i++)
        {
            pins[i] = pin_array[i];
        }
    }

    /**
     * デストラクタ
     */
    ~Sensor()
    {
        if (pins != nullptr)
        {
            delete[] pins;
            pins = nullptr;
        }
    }

    // コピーコンストラクタとコピー代入演算子を削除（ダブルデリート防止）
    Sensor(const Sensor &) = delete;
    Sensor &operator=(const Sensor &) = delete;

    /**
     * センサーピンを初期化（INPUT モード設定）
     */
    void begin()
    {
        if (pins == nullptr || count <= 0)
            return;

        for (int i = 0; i < count; i++)
        {
            pinMode(pins[i], INPUT);
        }
    }

    /**
     * 全センサーの値を読み込む（互換性のため残すが何もしない）
     */
    void read()
    {
        // cacheを削除したため、何もしない
        // state()で直接analogRead()を呼び出す
    }

    /**
     * センサー状態を 6ビットのビットマップで取得
     * @return 6ビット値（ビット i が 1 = センサー i で黒検出）
     */
    int state()
    {
        if (pins == nullptr || count <= 0)
            return 0;

        int result = 0;
        for (int i = 0; i < count; i++)
        {
            int value = analogRead(pins[i]);
            if (value >= threshold)
            {
                result |= (1 << i);
            }
        }
        return result;
    }

    /**
     * 指定ピン番号の値を取得
     * @param pin_index ピン番号（0～count-1）
     * @return アナログ読み込み値
     */
    int get(int pin_index)
    {
        if (pin_index >= 0 && pin_index < count && pins != nullptr)
        {
            return analogRead(pins[pin_index]);
        }
        return 0;
    }

    /**
     * 指定ピン番号が黒線を検出しているかチェック
     * @param pin_index ピン番号（0～count-1）
     * @return true: 黒線検出、false: 白
     */
    bool isBlack(int pin_index)
    {
        if (pin_index >= 0 && pin_index < count && pins != nullptr)
        {
            return analogRead(pins[pin_index]) >= threshold;
        }
        return false;
    }

    /**
     * デバッグ用: 全センサーの生値を表示
     */
    void printRaw()
    {
        Serial.print("Sensor Raw: ");
        for (int i = 0; i < count; i++)
        {
            Serial.print(analogRead(pins[i]));
            Serial.print(" ");
        }
        Serial.println();
    }

    /**
     * デバッグ用: センサー状態をバイナリで表示
     */
    void printState()
    {
        int s = state();
        Serial.print("Sensor State: 0b");
        Serial.println(s, BIN);
    }
};

#endif // SENSOR_H
