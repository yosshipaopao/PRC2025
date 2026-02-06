#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

/**
 * DC モーター制御クラス
 * 方向と速度を PWM で制御
 */
class Motor
{
private:
    int left_pin;  // 前進制御ピン
    int right_pin; // 後進制御ピン
    int pwm_pin;   // PWM 速度制御ピン

public:
    /**
     * コンストラクタ
     * @param lp 左（前進）ピン番号
     * @param rp 右（後進）ピン番号
     * @param pp PWM ピン番号
     */
    Motor(int lp, int rp, int pp) : left_pin(lp), right_pin(rp), pwm_pin(pp) {}

    /**
     * モーターピンを初期化（OUTPUT モード設定）
     */
    void begin()
    {
        pinMode(left_pin, OUTPUT);
        pinMode(right_pin, OUTPUT);
        pinMode(pwm_pin, OUTPUT);
        stop();
    }

    /**
     * モーターを速度指定で動作
     * @param speed -255～+255（正: 前進, 負: 後進, 0: 停止）
     */
    void run(int speed)
    {
        // 範囲制約
        speed = constrain(speed, -255, 255);

        if (speed > 0)
        {
            // 前進
            digitalWrite(left_pin, HIGH);
            digitalWrite(right_pin, LOW);
            analogWrite(pwm_pin, speed);
        }
        else if (speed < 0)
        {
            // 後進
            digitalWrite(left_pin, LOW);
            digitalWrite(right_pin, HIGH);
            analogWrite(pwm_pin, -speed);
        }
        else
        {
            // 停止
            stop();
        }
    }

    /**
     * モーターを停止
     */
    void stop()
    {
        digitalWrite(left_pin, LOW);
        digitalWrite(right_pin, LOW);
        analogWrite(pwm_pin, 0);
    }
};

#endif // MOTOR_H
