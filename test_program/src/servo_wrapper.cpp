#include "servo_wrapper.h"

#if ENABLE_SERVO

ServoWrapper::ServoWrapper()
    : targetAngle(0),
      isTimedMode(false), startTime(0), duration(0), secondAngle(0)
{
}

void ServoWrapper::setPeriodHertz(int hertz)
{
    servo.setPeriodHertz(hertz);
}

void ServoWrapper::attach(int pin, int minPulseWidth, int maxPulseWidth)
{
    servo.attach(pin, minPulseWidth, maxPulseWidth);
}

void ServoWrapper::write(int angle)
{
    targetAngle = angle;
    isTimedMode = false; // 通常のwriteはタイマーモードを解除
}

void ServoWrapper::set(int angle1, int angle2, unsigned long durationMs)
{
    targetAngle = angle1;
    secondAngle = angle2;
    duration = durationMs;
    startTime = millis();
    isTimedMode = true;
}

void ServoWrapper::applyWrite()
{
    // タイマーモードの場合、時間経過をチェック
    if (isTimedMode)
    {
        unsigned long elapsed = millis() - startTime;
        if (elapsed >= duration)
        {
            // 時間経過: 第2角度に切り替え
            targetAngle = secondAngle;
            isTimedMode = false;
        }
    }

    // 常にサーボに書き込み
    servo.write(targetAngle);
}
#endif
