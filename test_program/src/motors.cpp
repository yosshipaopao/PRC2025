#include <Arduino.h>
#include "config.h"
#include "motors.h"

// Motor クラスの実装
Motor::Motor(int l, int r, int pwm) : pinL(l), pinR(r), pinPwm(pwm) {}

void Motor::setup()
{
    pinMode(pinL, OUTPUT);
    pinMode(pinR, OUTPUT);
    pinMode(pinPwm, OUTPUT);
}

void Motor::run(int speed)
{
    speed = constrain(speed, -255, 255);
    if (speed >= 0)
    {
        digitalWrite(pinL, HIGH);
        digitalWrite(pinR, LOW);
        analogWrite(pinPwm, speed);
    }
    else
    {
        digitalWrite(pinL, LOW);
        digitalWrite(pinR, HIGH);
        analogWrite(pinPwm, -speed);
    }
}

// Motors クラスの実装
Motors::Motors()
    : m1(Config::MOTOR1_PIN_L, Config::MOTOR1_PIN_R, Config::MOTOR1_PIN_PWM),
      m2(Config::MOTOR2_PIN_L, Config::MOTOR2_PIN_R, Config::MOTOR2_PIN_PWM),
      m3(Config::MOTOR3_PIN_L, Config::MOTOR3_PIN_R, Config::MOTOR3_PIN_PWM),
      m4(Config::MOTOR4_PIN_L, Config::MOTOR4_PIN_R, Config::MOTOR4_PIN_PWM)
{
}

Motors::Motors(int m1l, int m1r, int m1p,
               int m2l, int m2r, int m2p,
               int m3l, int m3r, int m3p,
               int m4l, int m4r, int m4p)
    : m1(m1l, m1r, m1p),
      m2(m2l, m2r, m2p),
      m3(m3l, m3r, m3p),
      m4(m4l, m4r, m4p)
{
}

void Motors::setup()
{
    m1.setup();
    m2.setup();
    m3.setup();
    m4.setup();
}

void Motors::set(int leftSpeed, int rightSpeed)
{
    leftSpeed = constrain(leftSpeed, -Config::MOTOR_MAX_SPEED, Config::MOTOR_MAX_SPEED);
    rightSpeed = constrain(rightSpeed, -Config::MOTOR_MAX_SPEED, Config::MOTOR_MAX_SPEED);

    m1.run(rightSpeed);
    m2.run(rightSpeed);
    m3.run(leftSpeed);
    m4.run(leftSpeed);
}
