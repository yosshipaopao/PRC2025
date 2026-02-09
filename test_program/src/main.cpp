#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include "HardwareSerial.h"

#include "config.h"
#include "debug.h"
#include "intersections.h"
#include "motors.h"
#include "pattern.h"
#include "sensors.h"
#include "servo_wrapper.h"

static float lastError = 0.0f;

const int *sensor_pins = Config::LINE_SENSOR_PINS;

const int minServoPulseWidth = 500;  // マイクロ秒単位
const int maxServoPulseWidth = 2400; // マイクロ秒単位

Motors motors(Config::MOTOR1_PIN_L, Config::MOTOR1_PIN_R, Config::MOTOR1_PIN_PWM,
              Config::MOTOR2_PIN_L, Config::MOTOR2_PIN_R, Config::MOTOR2_PIN_PWM,
              Config::MOTOR3_PIN_L, Config::MOTOR3_PIN_R, Config::MOTOR3_PIN_PWM,
              Config::MOTOR4_PIN_L, Config::MOTOR4_PIN_R, Config::MOTOR4_PIN_PWM);
Sensors sensors(sensor_pins,
                Config::LINE_SENSOR_COUNT,
                Config::LINE_ACTIVE_SENSOR_OFFSET,
                Config::LINE_ACTIVE_SENSOR_COUNT);
Pattern pattern(Config::PATTERN_PIN_A, Config::PATTERN_PIN_B, Config::PATTERN_PIN_C, Config::PATTERN_STABLE_COUNT);
Intersections intersections(sensors, motors);

ServoWrapper servo1;
ServoWrapper servo2;
ServoWrapper servo3;
ServoWrapper servo4;
HardwareSerial mp3Serial(1); // UART1
DFRobotDFPlayerMini mp3;

long start_time = 0;
long time_stoper_start_time = 0;
long ab_stop_start_time = 0;
int crossCount = 0;
bool fryingPanReleased = false;
int unsigned long lastCrossTime = 0;
const unsigned long CROSS_COOLDOWN_MS = 1000; // 1秒間のクールダウン

struct CrossAction
{
    int servo1Angle;
    int servo2Angle;
    int servo3Angle;
    int servo4Angle;
    int soundIndex;
    int ledState; // 1 = ON, 0 = OFF
};

const CrossAction crossActions[] = {
    {180, 0, 0, 0, -1, 0},   // 0 初期状態
    {180, 0, 0, 0, -1, 0},   // 1 A->B
    {180, 0, 0, 0, 1, 1},    // 2 B -> B'
    {5, 0, 0, 0, -1, 1},     // 3 B' -> C -> A'
    {5, 0, 180, 180, -1, 1}, // 4 A' -> A
    {5, 0, 180, 180, -1, 1},
    {5, 0, 180, 180, -1, 1},
    {5, 0, 180, 180, -1, 1},
};

const int crossActionsCount = sizeof(crossActions) / sizeof(crossActions[0]);

void handleCrossByCount(int count)
{
    int index = count;
    if (index < 0 || index == 0)
    {
        index = 0;
    }
    if (index >= crossActionsCount)
    {
        index = crossActionsCount - 1;
    }
    if (index == 0)
    {
        start_time = millis();
    }
    else if (index == 2)
    {
        ab_stop_start_time = millis();
    }
    else if (index == 4)
    {
        time_stoper_start_time = millis();
    }

    const CrossAction &action = crossActions[index];

    motors.set(0, 0);
    Debug::print("CROSS_COUNT=");
    Debug::println(count);

    if (action.soundIndex != -1)
    {
        mp3.play(action.soundIndex);
    }

    // LED control based on action
    if (action.ledState == 1)
    {
        digitalWrite(Config::LED_PIN, HIGH);
    }
    else
    {
        digitalWrite(Config::LED_PIN, LOW);
    }

    servo1.write(action.servo1Angle);
    servo2.write(action.servo2Angle);
    servo3.write(action.servo3Angle);
    servo4.write(action.servo4Angle);
}

void handlePattern(int detectedPattern)
{
    switch (detectedPattern)
    {
    case 1:
        crossCount = 0;
        fryingPanReleased = false;
        Debug::println("ForceReset: CrossCount -> 0");
        handleCrossByCount(crossCount);
        break;
    case 2:
        crossCount = 1;
        fryingPanReleased = false;
        Debug::println("ForceSet: CrossCount -> 1");
        handleCrossByCount(crossCount);
        break;
    case 3:
        crossCount = 2;
        fryingPanReleased = false;
        Debug::println("ForceSet: CrossCount -> 2");
        handleCrossByCount(crossCount);
        break;
    case 4:
        crossCount = 3;
        fryingPanReleased = true;
        Debug::println("ForceSet: CrossCount -> 3");
        handleCrossByCount(crossCount);
        break;
    case 5:
        crossCount = 4;
        fryingPanReleased = true;
        Debug::println("ForceSet: CrossCount -> 4");
        handleCrossByCount(crossCount);
        break;
    case 6:
        servo3.set(180, 0, 5000);
        break;
    case 7:
        Debug::println("PATTERN_7: All pins detected");
        break;
    default:
        break;
    }
}

void setup()
{
    Debug::begin(115200);
    Debug::setSensors(&sensors);
    pinMode(Config::LED_PIN, OUTPUT);
    digitalWrite(Config::LED_PIN, LOW);
    pattern.setup();
    motors.setup();
    sensors.setup();
    mp3Serial.begin(9600, SERIAL_8N1, 7, 8);
    servo1.setPeriodHertz(50);
    servo1.attach(Config::SERVO_PIN_1, minServoPulseWidth, maxServoPulseWidth);
    servo2.setPeriodHertz(50);
    servo2.attach(Config::SERVO_PIN_2, minServoPulseWidth, maxServoPulseWidth);
    servo3.setPeriodHertz(50);
    servo3.attach(Config::SERVO_PIN_3, minServoPulseWidth, maxServoPulseWidth);
    servo4.setPeriodHertz(50);
    servo4.attach(Config::SERVO_PIN_4, minServoPulseWidth, maxServoPulseWidth);

    while (!mp3.begin(mp3Serial))
    {
        Debug::println("MP3 Player not found, retrying...");
        delay(1000);
    }

    // wait a moment
    delay(500);

    // initialize servos to 0 degree
    servo1.write(0);
    servo2.write(0);
    servo3.write(0);
    servo4.write(0);
}

void loop()
{
    pattern.updateDetection();

    intersections.updateFlags();
    Debug::sensorStates(lastError);

    Intersections::DetectionType detection = intersections.consumeDetection();

    switch (detection)
    {
    case Intersections::CROSS:
    {
        unsigned long currentTime = millis();
        if (currentTime - lastCrossTime >= CROSS_COOLDOWN_MS)
        {
            crossCount++;
            lastCrossTime = currentTime;
            handleCrossByCount(crossCount);
            motors.set(Config::LINE_BASE_SPEED, Config::LINE_BASE_SPEED);
            delay(Config::LEFT_TURN_STABILIZE_MS);
        }
        return;
    }

    case Intersections::LEFT_T:
        Debug::println("LEFT_T");

        intersections.turnLeftUntilLine(lastError);
        break;

    case Intersections::ALL_ACTIVE:
        Debug::println("ALL_ACTIVE_LEFT");
        if (!fryingPanReleased)
        {
            fryingPanReleased = true;
            motors.set(0, 0);
            servo3.set(180, 0, 5000);
            int startTime = millis();
            while (millis() - startTime < 5000)
            {
                servo3.applyWrite();
                delay(5);
            }
        }
        intersections.turnLeftUntilLine(lastError);
        break;

    case Intersections::NONE:
    default:
        break;
    }

    float error = sensors.readLineError(lastError);
    float diff = error - lastError;
    lastError = error;

    int correction = (int)(Config::LINE_KP * error + Config::LINE_KD * diff);
    int leftSpeed = Config::LINE_BASE_SPEED + correction;
    int rightSpeed = Config::LINE_BASE_SPEED - correction;

    motors.set(leftSpeed, rightSpeed);

    if (pattern.hasDetected())
    {
        int detectedPattern = pattern.consumeDetected();
        handlePattern(detectedPattern);
    }

    while (
        (crossCount == 4 && millis() - start_time < 1000 * (60 * 2 + 20) && millis() - time_stoper_start_time > 1000 * 10) ||
        (crossCount == 2 && millis() - ab_stop_start_time > 1000 * 15 && millis() - ab_stop_start_time < 1000 * 27))
    {

        pattern.updateDetection();
        motors.set(0, 0);
        if (pattern.hasDetected())
        {
            int detectedPattern = pattern.consumeDetected();
            handlePattern(detectedPattern);
        }
        delay(5);
    }
    // サーボの更新を適用
    servo1.applyWrite();
    servo2.applyWrite();
    servo3.applyWrite();
    servo4.applyWrite();

    delay(5);
}
