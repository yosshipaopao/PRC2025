#include <Arduino.h>
#include <config.h>

// ラインレース用パラメータ
constexpr float BASE_SPEED = 200.0f; // ベース速度 (PWM 0-255)
constexpr float Kp = 2.0f;           // P制御の比例ゲイン
constexpr float Ki = 0.02f;          // I制御の積分ゲイン
constexpr float Kd = 1.5f;           // D制御の微分ゲイン
constexpr int SENSOR_READ_INTERVAL_MS = 10;

// PID制御用の変数
int last_error = 0;
int integral_error = 0;
unsigned long last_sensor_time = 0;
unsigned long last_debug_time = 0;

// センサー値の保存
int sensor_values[Config::LINE_ACTIVE_SENSOR_COUNT];
int line_position = 0; // -2:最左, 0:中央, 2:最右

/**
 * モーターの方向と速度を設定
 */
void set_motor_speed(int motor_pin_l, int motor_pin_r, int motor_pin_pwm, int speed)
{
    if (speed >= 0)
    { // 前進
        digitalWrite(motor_pin_l, LOW);
        digitalWrite(motor_pin_r, HIGH);
    }
    else
    { // 後進
        digitalWrite(motor_pin_l, HIGH);
        digitalWrite(motor_pin_r, LOW);
        speed = -speed;
    }
    analogWrite(motor_pin_pwm, constrain(speed, 0, 255));
}

/**
 * 4つのモーターの速度を設定
 */
void set_all_motors(int left_speed, int right_speed)
{
    // 左前・左後
    set_motor_speed(Config::MOTOR1_PIN_L, Config::MOTOR1_PIN_R, Config::MOTOR1_PIN_PWM, left_speed);
    set_motor_speed(Config::MOTOR2_PIN_L, Config::MOTOR2_PIN_R, Config::MOTOR2_PIN_PWM, left_speed);

    // 右前・右後
    set_motor_speed(Config::MOTOR3_PIN_L, Config::MOTOR3_PIN_R, Config::MOTOR3_PIN_PWM, right_speed);
    set_motor_speed(Config::MOTOR4_PIN_L, Config::MOTOR4_PIN_R, Config::MOTOR4_PIN_PWM, right_speed);
}

/**
 * ラインセンサーを読み込む
 */
void read_line_sensors()
{
    for (int i = 0; i < Config::LINE_ACTIVE_SENSOR_COUNT; i++)
    {
        int pin = Config::LINE_SENSOR_PINS[Config::LINE_ACTIVE_SENSOR_OFFSET + i];
        int raw_value = analogRead(pin);
        // 黒ラインは値が低い、背景は値が高い
        sensor_values[i] = (raw_value < Config::LINE_SENSOR_THRESHOLD) ? 1 : 0;
    }
}

/**
 * ラインの位置を計算（重み付けセンタロイド法）
 * 戻り値: -20〜20 (-20:最左, 0:中央, 20:最右)
 */
int calculate_line_position()
{
    int weighted_sum = 0;
    int sensor_sum = 0;

    // センサーの重み：-2, -1, 0, 1, 2
    for (int i = 0; i < Config::LINE_ACTIVE_SENSOR_COUNT; i++)
    {
        int weight = i - 2; // -2から2
        weighted_sum += sensor_values[i] * weight;
        sensor_sum += sensor_values[i];
    }

    if (sensor_sum == 0)
    {
        return last_error; // ラインが見つからない場合は前回の値を返す
    }

    return (weighted_sum * 10) / sensor_sum; // スケーリング
}

/**
 * PID制御を使ってラインをトレース
 */
void line_trace_control()
{
    unsigned long now = millis();

    // センサー読み込み
    if (now - last_sensor_time >= SENSOR_READ_INTERVAL_MS)
    {
        read_line_sensors();
        last_sensor_time = now;

        // ラインの位置を計算
        line_position = calculate_line_position();

        // PID制御
        int error = line_position; // 目標は0（中央）

        // 比例項
        float p_term = Kp * error;

        // 積分項
        integral_error += error;
        integral_error = constrain(integral_error, -100, 100); // アンチワインドアップ
        float i_term = Ki * integral_error;

        // 微分項
        float d_term = Kd * (error - last_error);

        // 操舵量
        float steering = p_term + i_term + d_term;
        steering = constrain(steering, -100.0f, 100.0f);

        last_error = error;

        // モーター速度を計算
        float left_speed = BASE_SPEED - steering;
        float right_speed = BASE_SPEED + steering;

        set_all_motors((int)left_speed, (int)right_speed);

        // デバッグ出力
        if (now - last_debug_time >= Config::DEBUG_SENSOR_PRINT_INTERVAL_MS)
        {
            Serial.print("Sensors: ");
            for (int i = 0; i < Config::LINE_ACTIVE_SENSOR_COUNT; i++)
            {
                Serial.print(sensor_values[i]);
            }
            Serial.print(" | Pos: ");
            Serial.print(line_position);
            Serial.print(" | L:");
            Serial.print((int)left_speed);
            Serial.print(" R:");
            Serial.println((int)right_speed);

            last_debug_time = now;
        }
    }
}

/**
 * 初期化処理
 */
void setup()
{
    Serial.begin(115200);
    delay(100);

    Serial.println("=== Line Trace Robot ===");

    // LED
    pinMode(Config::LED_PIN, OUTPUT);
    digitalWrite(Config::LED_PIN, LOW);

    // ラインセンサーピン
    for (int i = Config::LINE_ACTIVE_SENSOR_OFFSET; i < Config::LINE_SENSOR_COUNT; i++)
    {
        pinMode(Config::LINE_SENSOR_PINS[i], INPUT);
    }

    // モーターピン
    for (int motor_pin : {Config::MOTOR1_PIN_L, Config::MOTOR1_PIN_R, Config::MOTOR1_PIN_PWM,
                          Config::MOTOR2_PIN_L, Config::MOTOR2_PIN_R, Config::MOTOR2_PIN_PWM,
                          Config::MOTOR3_PIN_L, Config::MOTOR3_PIN_R, Config::MOTOR3_PIN_PWM,
                          Config::MOTOR4_PIN_L, Config::MOTOR4_PIN_R, Config::MOTOR4_PIN_PWM})
    {
        pinMode(motor_pin, OUTPUT);
    }

    // 全モーターを停止
    set_all_motors(0, 0);

    Serial.println("Setup complete. Starting line trace...");
    delay(1000);
}

/**
 * メイン処理ループ
 */
void loop()
{
    line_trace_control();
    delay(5);
}