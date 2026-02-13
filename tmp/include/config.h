#pragma once

#define ENABLE_SERVO 1 // 0:無効, 1:有効

namespace Config
{
    // LED用ピン定義
    constexpr int LED_PIN = 6;

    // Servo用ピン定義
    constexpr int SERVO_PIN_1 = 43;
    constexpr int SERVO_PIN_3 = 45;

    // パターン認識用ピン定義
    constexpr int PATTERN_PIN_A = 11;
    constexpr int PATTERN_PIN_B = 10;
    constexpr int PATTERN_PIN_C = 9;
    constexpr int PATTERN_STABLE_COUNT = 20; // 20回連続で同じパターン

    // モーターピン定義（左ピン, 右ピン, PWMピン）
    constexpr int MOTOR1_PIN_L = 19, MOTOR1_PIN_R = 20, MOTOR1_PIN_PWM = 18; // left motor (front)
    constexpr int MOTOR2_PIN_L = 21, MOTOR2_PIN_R = 26, MOTOR2_PIN_PWM = 33; // left motor (back)
    constexpr int MOTOR3_PIN_L = 38, MOTOR3_PIN_R = 37, MOTOR3_PIN_PWM = 39; // right motor (front)
    constexpr int MOTOR4_PIN_L = 35, MOTOR4_PIN_R = 36, MOTOR4_PIN_PWM = 34; // right motor (back)

    // センサーパラメータ
    constexpr int LINE_SENSOR_COUNT = 6;
    constexpr int LINE_ACTIVE_SENSOR_COUNT = 5; // 12番ピンは未使用
    constexpr int LINE_SENSOR_THRESHOLD = 4250;
    constexpr int DEBUG_SENSOR_PRINT_INTERVAL_MS = 50;
    constexpr int LINE_SENSOR_PINS[LINE_SENSOR_COUNT] = {12, 17, 16, 15, 14, 13};
    constexpr int LINE_ACTIVE_SENSOR_OFFSET = 1; // LINE_SENSOR_PINS[1..5] をアクティブとして使用
}
