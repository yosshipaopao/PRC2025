#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include "HardwareSerial.h"
#include "ESP32Servo.h"

#include "config.h"
#include "debug.h"
#include "intersections.h"
#include "motors.h"
#include "pattern.h"
#include "sensors.h"

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

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
HardwareSerial mp3Serial(1); // UART1
DFRobotDFPlayerMini mp3;

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

    // while (!mp3.begin(mp3Serial))
    //{
    //     Debug::println("MP3 Player not found, retrying...");
    //     delay(1000);
    // }

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
    if (pattern.hasDetected())
    {
        pattern.execute(pattern.consumeDetected());
        return;
    }

    intersections.updateFlags();
    Debug::sensorStates(lastError);

    if (intersections.consumeCrossDetected())
    {
        motors.set(0, 0);
        Debug::println("CROSS");
        // mp3.play(1);

        // LED blink 3 times
        for (int i = 0; i < 3; i++)
        {
            digitalWrite(Config::LED_PIN, HIGH);
            delay(100);
            digitalWrite(Config::LED_PIN, LOW);
            delay(100);
        }

        // move servo

        servo1.write(180);
        servo2.write(180);
        servo3.write(180);
        servo4.write(180);
        //  hold position
        delay(1000);
        // return to initial position
        servo1.write(0);
        servo2.write(0);
        servo3.write(0);
        servo4.write(0);

        delay(Config::CROSS_STOP_DURATION_MS);
        return;
    }
    if (intersections.consumeLeftTDetected())
    {
        Debug::println("LEFT_T");
        intersections.turnLeftUntilLine(lastError);
    }
    if (intersections.consumeAllActiveTurnLeft())
    {
        Debug::println("ALL_ACTIVE_LEFT");
        intersections.turnLeftUntilLine(lastError);
    }

    float error = sensors.readLineError(lastError);
    float diff = error - lastError;
    lastError = error;

    int correction = (int)(Config::LINE_KP * error + Config::LINE_KD * diff);
    int leftSpeed = Config::LINE_BASE_SPEED - correction;
    int rightSpeed = Config::LINE_BASE_SPEED + correction;

    motors.set(leftSpeed, rightSpeed);
    delay(5);
}
