/**
 * PRC2025 メインプログラム
 * ライントレーサーロボット制御
 *
 * ESP32S2 Development Board
 * PlatformIO + Arduino Framework
 */

#include <Arduino.h>
#include <HardwareSerial.h>
#include "motor.h"
#include "sensor.h"
#include "state_manager.h"
#include "timeout_manager.h"
#include "debug.h"

// ================================
// 定数定義
// ================================

// モーター速度定数
const int RUN_SP = 130;      // 通常走行速度
const int HALF_STOP_SP = 70; // ハーフストップ速度（軽く舵切り）
const int STOP_SP = 0;       // 停止

// タイミング定数
const unsigned long LOOP_INTERVAL = 5; // メインループ周期（ms）
const int C_detection_threshold = 6;   // イベント検出の状態継続フレーム数

// ピン定義
const int LED_PIN = 6;

// モーターピン定義（左ピン, 右ピン, PWMピン）
const int M1_L = 37, M1_R = 38, M1_P = 39;
const int M2_L = 21, M2_R = 26, M2_P = 33;
const int M3_L = 35, M3_R = 36, M3_P = 34;
const int M4_L = 20, M4_R = 19, M4_P = 18;

// センサーピン定義
int sensor_pins[] = {13, 14, 15, 16, 17, 12};
const int SENSOR_COUNT = 6;
const int SENSOR_THRESHOLD = 2000;

// ================================
// グローバル変数
// ================================

// ハードウェア制御インスタンス
Motor m1(M1_L, M1_R, M1_P);
Motor m2(M2_L, M2_R, M2_P);
Motor m3(M3_L, M3_R, M3_P);
Motor m4(M4_L, M4_R, M4_P);

Sensor sensor(sensor_pins, SENSOR_COUNT, SENSOR_THRESHOLD);
StateManager state_manager;
TimeoutManager timeout_manager;

// カウンタ・タイミング変数
int cross_count = 0;            // 十字交差点カウンタ
unsigned long nextLoopTime = 0; // 次のループ実行時刻
// 前回のモーター速度（defaultケース用）
int prev_left_speed = RUN_SP;
int prev_right_speed = RUN_SP;
// ================================
// 状態マップテーブル
// ================================

StateMapEntry state_map[] = {
    // Group 3: ラインなし（最優先でチェック）
    {0b000000, 3}, // すべて白

    // Group 4: 全センサー（十字）
    {0b011111, 4}, // センサー0,1,2,3,4
    {0b111111, 4}, // すべて黒（念のため）

    // Group 0: 中央（直進）
    {0b000100, 0}, // センサー2のみ
    {0b001110, 0}, // センサー1,2,3
    {0b001010, 0}, // センサー1,3
    {0b001100, 0}, // センサー2,3
    {0b011110, 0}, // センサー1,2,3,4
    {0b001101, 0}, // センサー0,2,3
    {0b010110, 0}, // センサー1,2,4
    {0b001000, 0}, // センサー3のみ
    {0b010000, 0}, // センサー4のみ

    // Group 1: 左寄り（センサー12を含む）
    //{0b100001, 1}, // センサー4,12(5)
    {0b110001, 1}, // センサー4,12(5)
    {0b111001, 1}, // センサー3,4,12(5)
    {0b111101, 1}, // センサー2,3,4,12(5)

    // Group 2: 右寄り
    {0b000010, 2}, // センサー0,1
    {0b000110, 2}, // センサー0,1,2
    {0b001110, 2}, // センサー0,1,2,3
};

const int STATE_MAP_SIZE = sizeof(state_map) / sizeof(StateMapEntry);

// ================================
// モーター制御関数
// ================================

/**
 * モーターを停止
 */
void stop_motor()
{
    m1.stop();
    m2.stop();
    m3.stop();
    m4.stop();
}

/**
 * モーター速度を設定（スムージング処理前）
 * @param left_speed 左側モーター速度
 * @param right_speed 右側モーター速度
 */
void run_motor(int left, int right)
{
    m1.run(left);
    m2.run(left);
    m3.run(right);
    m4.run(right);
}

// ================================
// setup 関数
// ================================

void setup()
{
    // 安定化待機
    delay(2000);

    // シリアル通信初期化（デバッグ用）
    Debug::begin(115200);
    Debug::println("PRC2025 Program Starting...");

    // LED ピン設定
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // モーター初期化
    m1.begin();
    m2.begin();
    m3.begin();
    m4.begin();

    // センサー初期化
    sensor.begin();

    // 状態マップ設定
    state_manager.setStateMap(state_map, STATE_MAP_SIZE);

    // 初期化完了を示すLED点滅
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }

    // メインループ基準時刻記録
    nextLoopTime = millis();

    Debug::println("Initialization Complete!");

    // state_map の内容を出力
    Debug::println("State Map Contents:");
    for (int i = 0; i < STATE_MAP_SIZE; i++)
    {
        Debug::print("  [");
        Debug::print(i);
        Debug::print("] 0b");
        for (int b = 5; b >= 0; b--)
        {
            Debug::print((state_map[i].sensor_state & (1 << b)) ? "1" : "0");
        }
        Debug::print(" (");
        Debug::print(state_map[i].sensor_state);
        Debug::print(") -> Group ");
        Debug::println(state_map[i].group_id);
    }

    Debug::println("Starting main loop...");
}

// ================================
// loop 関数
// ================================

void loop()
{
    // タイミング同期
    /*
    unsigned long currentTime = millis();
    long time_diff = (long)(nextLoopTime - currentTime);

    // 100ms 以上遅延、または大幅に進んでいる場合は同期をリセット
    if (time_diff > 100 || time_diff < -1000)
    {
        nextLoopTime = currentTime;
    }

    // 次のループ6時刻まで待機
    while ((long)(millis() - nextLoopTime) < 0)
    {
        delayMicroseconds(100);
    }
    nextLoopTime += LOOP_INTERVAL;
    */  
    delay(LOOP_INTERVAL); // 簡易版タイミング同期

    // ================================
    // センサー読み込み
    // ================================
    sensor.read();
    int sensor_state = sensor.state();

    // 6センサー全てを使用

    // ================================
    // 状態管理と遷移検出
    // ================================
    // デバッグ用：mapStateの直接呼び出し結果を確認
    int mapped_result = state_manager.mapState(sensor_state);

    bool state_changed = state_manager.updateState(sensor_state);
    int current_state = state_manager.getCurrentState();
    int prev_state = state_manager.getPrevState();
    int prev_state_count = state_manager.getPrevStateCount();
    int current_state_count = state_manager.getCurrentStateCount();

    // デバッグ出力（状態変化時）
    if (state_changed)
    {
        Debug::print("State Changed: ");
        Debug::print(prev_state);
        Debug::print(" (");
        Debug::print(prev_state_count);
        Debug::print("f) -> ");
        Debug::println(current_state);
    }

    // デバッグ出力（現在のセンサー状況）
    if (timeout_manager.get(1) == false)
    {
        Debug::print("Current State: ");
        for (int i = 5; i >= 0; i--)
        {
            Debug::print((sensor_state & (1 << i)) ? "1" : "0");
        }
        Debug::print(" (");
        Debug::print(sensor_state);
        Debug::print(") | mapState: ");
        Debug::print(mapped_result);
        Debug::print(" | Mapped State: ");
        Debug::print(current_state);
        Debug::print(" (");
        Debug::print(current_state_count);
        Debug::println("f)");
    }

    // ================================
    // イベント検出
    // ================================

    // C字折り返し検出
    if (current_state == 1 &&
        current_state_count >= C_detection_threshold &&
        timeout_manager.get(0))
    {

        Debug::println(">>> C-Turn Detected! Turning Left...");

        // 左旋回実行
        prev_left_speed = -HALF_STOP_SP;
        prev_right_speed = RUN_SP;
        run_motor(prev_left_speed, prev_right_speed);
        delay(1000);
        // タイムアウト設定
        timeout_manager.set(0, 3000);
    }

    // C字復帰検出（行き止まり）
    if (current_state == 3 &&
        current_state_count >= C_detection_threshold * 12 &&
        timeout_manager.get(1) &&
        timeout_manager.get(0))
    {

        Debug::println(">>> Dead End Detected! Turning Right...");

        // 左旋回実行
        prev_left_speed = -RUN_SP;
        prev_right_speed = RUN_SP;
        run_motor(prev_left_speed, prev_right_speed);
        delay(1000);

        // タイムアウト設定
        timeout_manager.set(1, 3000);
    }

    // 十字交差点検出
    if (state_changed &&
        prev_state == 4 &&
        (current_state == 0 || current_state == -1) &&
        prev_state_count >= C_detection_threshold &&
        timeout_manager.get(3))
    {

        Debug::println(">>> Cross Intersection Detected!");

        // 交差点カウンタ増加
        cross_count++;
        Debug::print("Cross Count: ");
        Debug::println(cross_count);

        // タイムアウト設定
        timeout_manager.set(3, 3000);
    }

    // ================================
    // ライントレーサー制御
    // ================================

    int left_speed = RUN_SP;
    int right_speed = RUN_SP;

    // 各センサーの状態を取得（6センサー全て）
    bool s0 = sensor.isBlack(0); // 右端
    bool s1 = sensor.isBlack(1);
    bool s2 = sensor.isBlack(2); // 中央
    bool s3 = sensor.isBlack(3);
    bool s4 = sensor.isBlack(4); // 左端
    bool s5 = sensor.isBlack(5); // センサー12（C detect）

    // ライントレースロジック（if文ベース）
    if (s0 && s1 && s2 && s3 && s4)
    {
        // すべてのセンサー反応（十字付近） - 直進
        left_speed = RUN_SP;
        right_speed = RUN_SP;
    }
    else if (s2 && !s0 && !s4)
    {
        // 中央センサー反応（直進）
        // センサー2が反応していて、両端（0,4）が反応していない
        left_speed = RUN_SP;
        right_speed = RUN_SP;
    }
    else if (s3 && !s4)
    {
        // センサー3が反応、センサー4は反応していない（左への軽い舵切り）
        left_speed = HALF_STOP_SP;
        right_speed = RUN_SP;
    }
    else if (s1 && !s0)
    {
        // センサー1が反応、センサー0は反応していない（右への軽い舵切り）
        left_speed = RUN_SP;
        right_speed = HALF_STOP_SP;
    }
    else if (s4 && !s3)
    {
        // センサー4のみ、またはセンサー4が主（左旋回）
        left_speed = -RUN_SP;
        right_speed = RUN_SP;
    }
    else if (s0 && !s1)
    {
        // センサー0のみ、またはセンサー0が主（右旋回）
        left_speed = RUN_SP;
        right_speed = -RUN_SP;
    }
    else if (s4)
    {
        // センサー4が反応（左への強い舵切りまたは左旋回）
        if (s3 && s2)
        {
            // センサー2,3,4が反応（左旋回）
            left_speed = -RUN_SP;
            right_speed = RUN_SP;
        }
        else
        {
            // センサー4が反応（左への強い舵切り）
            left_speed = STOP_SP;
            right_speed = RUN_SP;
        }
    }
    else if (s0)
    {
        // センサー0が反応（右への強い舵切りまたは右旋回）
        if (s1 && s2)
        {
            // センサー0,1,2が反応（右旋回）
            left_speed = RUN_SP;
            right_speed = -RUN_SP;
        }
        else
        {
            // センサー0が反応（右への強い舵切り）
            left_speed = RUN_SP;
            right_speed = STOP_SP;
        }
    }
    else
    {
        // その他のパターン（前回の速度を維持）
        left_speed = prev_left_speed;
        right_speed = prev_right_speed;
    }

    // ================================
    // モーター実行
    // ================================

    // ラインを検知している場合のみ速度を更新
    // if (line_sensor_state != 0b000000)
    //{
    //    run_motor(left_speed, right_speed);
    //    prev_left_speed = left_speed;
    //    prev_right_speed = right_speed;
    //}

    m1.run(left_speed);
    m2.run(left_speed);
    m3.run(right_speed);
    m4.run(right_speed);

    // Debug::print("Motor Speeds Set: L=");
    // Debug::print(motorPowerAverager.getL());
    // Debug::print(" R=");
    // Debug::println(motorPowerAverager.getR());

    // 今回の速度を保存（次回のdefaultケース用）
}
