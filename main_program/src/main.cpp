#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include "motor.h"
#include "sensor.h"

HardwareSerial mp3Serial(1); // UART1
DFRobotDFPlayerMini mp3;
const int M1_P = 39, M1_L = 37, M1_R = 38, M2_R = 26, M2_L = 21, M2_P = 33;
const int M3_P = 34, M3_L = 35, M3_R = 36, M4_R = 19, M4_L = 20, M4_P = 18;
const int sensor_pins[] = {13, 14, 15, 16, 17};
Motor m1(M1_L, M1_R, M1_P);
Motor m2(M2_L, M2_R, M2_P);
Motor m3(M3_L, M3_R, M3_P);
Motor m4(M4_L, M4_R, M4_P);
Sensor sensor((int *)sensor_pins, 5, 2000);

const int RUN_SP = 100;
const int HALF_STOP_SP = 50;
const int STOP_SP = -20;

// -255 ~ 255
void run_motor(int left_speed, int right_speed)
{
  m1.run(left_speed);
  m2.run(left_speed);
  m3.run(right_speed);
  m4.run(right_speed);
}

void stop_motor()
{
  m1.stop();
  m2.stop();
  m3.stop();
  m4.stop();
}

void setup()
{
  delay(2000);

  Serial.begin(9600);
  Serial.println("Starting...");

  pinMode(6, OUTPUT);
  m1.setup();
  m2.setup();
  m3.setup();
  m4.setup();
  sensor.setup();
}

void loop()
{
  sensor.read();

  // Line Tracer Logic
  switch (sensor.state())
  {
  case 0b11111:
  case 0b01110:
  case 0b00100:
    run_motor(RUN_SP, RUN_SP);
    break;
  case 0b01100:
    run_motor(RUN_SP, HALF_STOP_SP);
    break;
  case 0b00110:
    run_motor(HALF_STOP_SP, RUN_SP);
    break;
  case 0b10000:
    run_motor(RUN_SP, STOP_SP);
    break;
  case 0b00001:
    run_motor(STOP_SP, RUN_SP);
    break;
  case 0b11000:
    run_motor(RUN_SP, STOP_SP);
    break;
  case 0b11100:
    run_motor(RUN_SP, STOP_SP);
    break;
  case 0b11110:
    run_motor(HALF_STOP_SP, RUN_SP);
    break;
  case 0b00011:
    run_motor(STOP_SP, RUN_SP);
    break;
  case 0b00111:
    run_motor(STOP_SP, RUN_SP);
    break;
  case 0b01111:
    run_motor(HALF_STOP_SP, RUN_SP);
    break;
  default:
    run_motor(RUN_SP, RUN_SP);
    break;
  }

  delay(10);
}
