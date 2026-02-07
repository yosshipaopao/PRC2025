#pragma once

// デバッグ出力設定
#define ENABLE_SENSOR_PRINT 0 // 0:無効, 1:有効
#define ENABLE_SERVO 1        // 0:無効, 1:有効

namespace Config
{
    // LED用ピン定義
    constexpr int LED_PIN = 6;

    // Servo用ピン定義
    constexpr int SERVO_PIN_1 = 43;
    constexpr int SERVO_PIN_2 = 44;
    constexpr int SERVO_PIN_3 = 45;
    constexpr int SERVO_PIN_4 = 46;

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
    constexpr int LINE_SENSOR_THRESHOLD = 2000;
    constexpr int DEBUG_SENSOR_PRINT_INTERVAL_MS = 50;
    constexpr int LINE_SENSOR_PINS[LINE_SENSOR_COUNT] = {12, 17, 16, 15, 14, 13};
    constexpr int LINE_ACTIVE_SENSOR_OFFSET = 1; // LINE_SENSOR_PINS[1..5] をアクティブとして使用

    // ライントレース制御パラメータ
    constexpr int LINE_BASE_SPEED = 110; // 0-255
    constexpr int MOTOR_MAX_SPEED = 220; // 0-255
    constexpr float LINE_KP = 45.0f;     // 比例ゲイン
    constexpr float LINE_KD = 25.0f;     // 微分ゲイン
    constexpr float LINE_ERROR_DEADBAND = 0.10f;

    // 交差検知用
    constexpr int LINE_FAR_LEFT_INDEX = 0; // line_sensor_pins の 12番
    constexpr int LINE_LEFT_INDEX = 0;     // active_line_sensors の左端
    constexpr int LINE_MID_LEFT_INDEX = 1;
    constexpr int LINE_CENTER_INDEX = 2; // active_line_sensors の中央
    constexpr int LINE_MID_RIGHT_INDEX = 3;
    constexpr int LINE_RIGHT_INDEX = 4; // active_line_sensors の右端
    constexpr int CROSS_STOP_DURATION_MS = 1200;
    constexpr int LEFT_TURN_SPEED = 140;
    constexpr int LEFT_TURN_SPEED_SLOW = 80;
    constexpr int LEFT_TURN_OVERRUN_MS = 400;
    constexpr int LEFT_TURN_STABILIZE_MS = 200;
    constexpr int INTERSECTION_STABLE_COUNT = 5;
    constexpr int ALL_ACTIVE_TIMEOUT_MS = 50;
    constexpr int CROSS_DETECT_WINDOW_MS = 300;
}
