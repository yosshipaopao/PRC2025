#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include "motor.h"
#include "sensor.h"

HardwareSerial mp3Serial(1); // UART1
DFRobotDFPlayerMini mp3;
const int M1_P = 33, M1_L = 34, M1_R = 35, M2_R = 36, M2_L = 37, M2_P = 38;
const int M3_P = 39, M3_L = 40, M3_R = 41, M4_R = 42, M4_L = 43, M4_P = 44;
const int sensor_pins[] = {13, 14, 15, 16, 17};
Motor m1(M1_L, M1_R, M1_P);
Motor m2(M2_L, M2_R, M2_P);
Motor m3(M3_L, M3_R, M3_P);
Motor m4(M4_L, M4_R, M4_P);
Sensor sensor((int *)sensor_pins, 5, 2000);

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
  //mp3Serial.begin(9600, SERIAL_8N1, 7, 8);
/*
  while (!mp3.begin(mp3Serial))
  {
    Serial.println("DFPlayer Mini not detected!");
    delay(1000);
  }
  mp3.volume(30); // Set volume value (0~30).
  */
}

void loop()
{
  sensor.read();

  // sensor.isBlack(i) -> bool
  sensor.print_raw();

  // Line Tracer Logic
  if (sensor.isBlack(0) && !sensor.isBlack(4))
  {
    // Turn Right
    run_motor(150, 255);
  }
  else if (!sensor.isBlack(0) && sensor.isBlack(4))
  {
    // Turn Left
    run_motor(255, 150);
  }
  else if (sensor.isBlack(0) && sensor.isBlack(4))
  {
    // Move Forward
    run_motor(255, 255);
  }
  else
  {
    // Stop
    stop_motor();
  }
  

  delay(10);
}
