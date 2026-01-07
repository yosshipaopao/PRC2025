#ifndef _motor_h
#define _motor_h

#include <Arduino.h>

class Motor
{
public:
    Motor(int left, int right, int pwm);
    void setup();
    void run(int speed);
    void stop();

private:
    void forward(int speed);
    void backward(int speed);
    int left, right, pwm;
};

Motor::Motor(int left, int right, int pwm)
{
    this->left = left;
    this->right = right;
    this->pwm = pwm;
}

void Motor::setup()
{
    pinMode(left, OUTPUT);
    pinMode(right, OUTPUT);
    pinMode(pwm, OUTPUT);
}

void Motor::run(int speed)
{
    speed = constrain(speed, -255, 255);
    if (speed > 0)
        forward(speed);
    else if (speed < 0)
        backward(-speed);
    else
        stop();
}

void Motor::forward(int speed)
{
    digitalWrite(left, HIGH);
    digitalWrite(right, LOW);
    analogWrite(pwm, speed);
}

void Motor::backward(int speed)
{
    digitalWrite(left, LOW);
    digitalWrite(right, HIGH);
    analogWrite(pwm, speed);
}

void Motor::stop()
{
    digitalWrite(left, LOW);
    digitalWrite(right, LOW);
    analogWrite(pwm, 0);
}

#endif