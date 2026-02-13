#include <Arduino.h>
#include <cmath>
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

ServoWrapper servo_arm;
ServoWrapper servo_hand;
HardwareSerial mp3Serial(1); // UART1
DFRobotDFPlayerMini mp3;

long start_time = 0;
long cross_start_time = 0; // Start time for current crossCount
int crossCount = 0;
bool fryingPanReleased = false;
int unsigned long lastCrossTime = 0;
const unsigned long CROSS_COOLDOWN_MS = 1000; // 1秒間のクールダウン

struct CrossAction
{
    int servo_arm_angle;
    int servo_hand_angle;
    int soundIndex;
    int ledState; // 1 = ON, 0 = OFF
};

const CrossAction crossActions[] = {
    {180, 0, -1, 1}, // 0 初期状態
    {180, 0, -1, 1}, // 1 A->B
    {180, 0, 1, 1},  // 2 B -> B'
    {5, 0, -1, 1},   // 3 B' -> C -> A'
    {5, 180, -1, 0}, // 4 A' -> A
    {5, 180, -1, 0},
    {5, 180, -1, 0},
    {5, 180, -1, 0},
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
    if (index >= 2 && index <= 4)
    {
        cross_start_time = millis();
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

    servo_arm.write(action.servo_arm_angle);
    servo_hand.write(action.servo_hand_angle);
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
        fryingPanReleased = false;
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
        servo_hand.set(180, 0, 5000);
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
    servo_arm.setPeriodHertz(50);
    servo_arm.attach(Config::SERVO_PIN_1, minServoPulseWidth, maxServoPulseWidth);
    servo_hand.setPeriodHertz(50);
    servo_hand.attach(Config::SERVO_PIN_3, minServoPulseWidth, maxServoPulseWidth);

    while (!mp3.begin(mp3Serial))
    {
        Debug::println("MP3 Player not found, retrying...");
        delay(1000);
    }

    // wait a moment
    delay(500);

    // initialize servos to 0 degree
    servo_arm.write(0);
    servo_hand.write(0);
}

void loop()
{
    pattern.updateDetection();

    intersections.updateFlags();
    Debug::sensorStates(lastError);
    // Debug::printSensorRaw();

    Intersections::DetectionType detection = intersections.consumeDetection();

    switch (detection)
    {
    case Intersections::CROSS:
    {
        Debug::println("CROSS_DETECTED");
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
            servo_hand.set(180, 0, 5000);
            unsigned long startTime = millis();
            while (millis() - startTime < 5000)
            {
                servo_hand.applyWrite();
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

    // Apply servo_arm shake effect for crossCount == 3 within 10 seconds
    if (crossCount == 3 && (millis() - cross_start_time < 5 * 1000))
    {
        motors.set(0, 0);
        unsigned long elapsed = millis() - cross_start_time;
        float cycle = (elapsed % 1000) / 1000.0f; // 0-1 cycle over 1 second
        float shakeAngle = 180 + 30 * sin(2.0f * M_PI * cycle);
        servo_arm.write((int)shakeAngle);
        servo_arm.applyWrite();
        delay(5);
        return;
    }
    // Rotate servo_arm from 180 to 5 over 1 second with sin curve after 10 seconds when crossCount == 3
    if (crossCount == 3 && (millis() - cross_start_time >= 5 * 1000) && (millis() - cross_start_time < 6 * 1000))
    {
        motors.set(0, 0);
        unsigned long elapsed = millis() - cross_start_time - 5000;
        float progress = elapsed / 1000.0f; // 0-1 over 1 second
        // Use sin curve for smooth rotation (easing-out effect)
        float smoothProgress = sin(progress * M_PI / 2.0f);
        float rotationAngle = 180 + (5 - 180) * smoothProgress;
        servo_arm.write((int)rotationAngle);
        servo_arm.applyWrite();
        delay(5);
        return;
    }

    while (
        (crossCount == 4 && millis() - start_time < 1000 * (60 * 2 + 20) && millis() - cross_start_time > 1000 * 10) ||
        (crossCount == 2 && millis() - cross_start_time > 1000 * 15 && millis() - cross_start_time < 1000 * 32))
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
    servo_arm.applyWrite();
    servo_hand.applyWrite();

    delay(5);
}
