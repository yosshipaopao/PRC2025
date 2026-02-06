#pragma once

class Motor
{
private:
    int pinL, pinR, pinPwm;

public:
    Motor(int l, int r, int pwm);
    void setup();
    void run(int speed);
};

class Motors
{
private:
    Motor m1, m2, m3, m4;

public:
    Motors();
    Motors(int m1l, int m1r, int m1p,
           int m2l, int m2r, int m2p,
           int m3l, int m3r, int m3p,
           int m4l, int m4r, int m4p);
    void setup();
    void set(int leftSpeed, int rightSpeed);
};
