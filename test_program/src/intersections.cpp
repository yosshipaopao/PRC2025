#include <Arduino.h>
#include "config.h"
#include "debug.h"
#include "intersections.h"
#include "motors.h"
#include "sensors.h"

/**
 * @brief Intersectionsコンストラクタ
 * @param s センサーオブジェクトへの参照
 * @param m モーターオブジェクトへの参照
 */
Intersections::Intersections(Sensors &s, Motors &m)
    : sensors(s), motors(m), crossDetected(false), leftTDetected(false),
      allActiveDetected(false), allActiveTurnLeft(false), leftTCount(0),
      allActiveCount(0), allActiveDetectedMs(0)
{
    // CROSS検知用タイムスタンプの初期化
    for (int i = 0; i < 6; i++)
    {
        crossSensorFlags[i] = 0;
    }
}

/**
 * @brief 各交差パターンの検知フラグを更新
 *
 * 以下の3つの交差パターンを検知:
 * 1. CROSS: 全6個のセンサーが検知（+字路）
 * 2. LEFT_T: 左T字路（左3個 + 中央のみ検知）
 * 3. ALL_ACTIVE: 全5個のアクティブセンサーが検知（T字路の判定）
 */
void Intersections::updateFlags()
{
    // 各センサーの検知状態を読み込む
    bool farLeft = sensors.isLineDetectedByIndex(Config::LINE_FAR_LEFT_INDEX);
    bool left = sensors.isActiveLineDetected(Config::LINE_LEFT_INDEX);
    bool midLeft = sensors.isActiveLineDetected(Config::LINE_MID_LEFT_INDEX);
    bool center = sensors.isActiveLineDetected(Config::LINE_CENTER_INDEX);
    bool midRight = sensors.isActiveLineDetected(Config::LINE_MID_RIGHT_INDEX);
    bool right = sensors.isActiveLineDetected(Config::LINE_RIGHT_INDEX);

    // 各交差パターンの検知判定
    bool cross = farLeft && left && midLeft && center && midRight && right;   // +字路
    bool leftT = farLeft && left && midLeft && center && !midRight && !right; // 左T字路
    bool allActive = left && midLeft && center && midRight && right;          // 5個センサー全て

    // 連続検知カウントを更新（安定化判定用）
    leftTCount = leftT ? (leftTCount + 1) : 0;
    allActiveCount = allActive ? (allActiveCount + 1) : 0;

    // ===== +字路（CROSS）の検知処理 =====
    // 毎フレーム各センサーが検知されるたびにタイムスタンプを更新
    if (farLeft)
        crossSensorFlags[0] = millis();
    if (left)
        crossSensorFlags[1] = millis();
    if (midLeft)
        crossSensorFlags[2] = millis();
    if (center)
        crossSensorFlags[3] = millis();
    if (midRight)
        crossSensorFlags[4] = millis();
    if (right)
        crossSensorFlags[5] = millis();

    // ウィンドウ内に全センサーが一度でも検知されたかを確認
    if (!crossDetected)
    {
        bool allDetectedInWindow = true;
        unsigned long now = millis();

        for (int i = 0; i < 6; i++)
        {
            // タイムスタンプが記録されていない、または時間が経ちすぎている
            if (crossSensorFlags[i] == 0 || (now - crossSensorFlags[i] > Config::CROSS_DETECT_WINDOW_MS))
            {
                allDetectedInWindow = false;
                break;
            }
        }

        bool anyCenterDetected = (midLeft || center || midRight);
        bool othersOff = (!farLeft && !left && !right);

        if (allDetectedInWindow && anyCenterDetected && othersOff)
        {
            crossDetected = true;
            Debug::println("CROSS_CONFIRMED");
            // フラグをリセット
            for (int i = 0; i < 6; i++)
            {
                crossSensorFlags[i] = 0;
            }
        }
    }

    // ===== 左T字路（LEFT_T）の検知処理 =====
    // +字路が検知されていない且つ一定時間連続検知で確定
    if (!leftTDetected)
    {
        if (
            (leftTCount >= Config::INTERSECTION_STABLE_COUNT) &&
            crossSensorFlags[5] <= Config::CROSS_DETECT_WINDOW_MS)
        {
            leftTDetected = true;
            Debug::println("LEFT_T_DETECTED");
        }
    }

    // ===== 5個センサー全検知時の処理 =====
    // +字路でも左T字路でもない場合に検知開始
    if ((!crossDetected && !leftTDetected) && (allActiveCount >= Config::INTERSECTION_STABLE_COUNT))
    {
        if (!allActiveDetected)
        {
            allActiveDetected = true;
            Debug::println("ALL_ACTIVE_START");
        }
        // allActive継続中はタイムスタンプを更新し続ける
        allActiveDetectedMs = millis();
    }
    // 5個センサー検知中の処理
    else if (allActiveDetected)
    {
        // 全センサーオフで左折トリガー
        bool allOff = !farLeft && !left && !midLeft && !center && !midRight && !right;

        // タイムアウト: allActive終了後一定時間経過したらリセット
        if (millis() - allActiveDetectedMs > Config::ALL_ACTIVE_TIMEOUT_MS)
        {
            allActiveDetected = false;
            allActiveCount = 0;
        }
        // センサーオフで左折を実行
        else if (allOff)
        {
            allActiveTurnLeft = true;
            allActiveDetected = false;
            allActiveCount = 0;
            Debug::println("ALL_ACTIVE_TURN_TRIGGER");
        }
    }
}

/**
 * @brief 検知した交差パターンを消費して返す
 * @return 検知されたパターン（CROSS, LEFT_T, ALL_ACTIVE, NONE）
 *
 * 優先度順で検知フラグをチェックし、対応するパターンを返す
 * 呼び出されたタイミングでフラグをリセット
 */
Intersections::DetectionType Intersections::consumeDetection()
{
    // 優先度1: +字路（CROSS）
    if (crossDetected)
    {
        crossDetected = false;
        return Intersections::CROSS;
    }

    // 優先度2: 左T字路（LEFT_T）
    if (leftTDetected)
    {
        leftTDetected = false;
        leftTCount = 0;
        return Intersections::LEFT_T;
    }

    // 優先度3: 5個センサー全検知（ALL_ACTIVE）
    if (allActiveTurnLeft)
    {
        allActiveTurnLeft = false;
        allActiveDetected = false;
        allActiveCount = 0;
        return Intersections::ALL_ACTIVE;
    }

    // 何も検知されていない
    return Intersections::NONE;
}

/**
 * @brief 左折処理を実行（全センサー制御）
 * @param lastError 直前のエラー値（参照で更新される）
 *
 * 処理の流れ:
 * 1. オーバーラン: 直進して交差点を抜ける
 * 2. 回転: 中央センサーがラインを失うまで左旋回
 * 3. 再検知: 中央センサーがラインを再検知するまで低速左旋回
 * 4. エラー値を左方向バイアスに設定
 */
void Intersections::turnLeftUntilLine(float &lastError)
{
    // ===== ステップ1: オーバーラン（交差点を抜ける） =====
    motors.set(Config::LINE_BASE_SPEED, Config::LINE_BASE_SPEED);
    delay(Config::LEFT_TURN_OVERRUN_MS);

    // ===== ステップ2: 回転（中央センサーがラインから外れるまで） =====
    // 左後退 + 右前進 = 左旋回
    motors.set(-Config::LEFT_TURN_SPEED, Config::LEFT_TURN_SPEED);
    while (sensors.isActiveLineDetected(Config::LINE_CENTER_INDEX))
    {
        Debug::sensorStates(lastError);
        delay(5);
    }

    // ===== 安定化待ち（旋回時の過渡応答を待つ） =====
    delay(Config::LEFT_TURN_STABILIZE_MS);

    // ===== ステップ3: 再検知（低速で中央センサーがラインを再検知するまで） =====
    motors.set(-Config::LEFT_TURN_SPEED_SLOW, Config::LEFT_TURN_SPEED_SLOW);
    while (!sensors.isActiveLineDetected(Config::LINE_CENTER_INDEX))
    {
        Debug::sensorStates(lastError);
        delay(5);
    }

    // ===== エラー値を左方向バイアスに設定 =====
    // PD制御で左側への旋回傾向を持たせる
    lastError = -1.5f;
}
