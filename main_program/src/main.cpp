#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include "motor.h"
#include "sensor.h"
#include "motor_power_averager.h"
#include "state_manager.h"
#include "timeout_manager.h"

const int LED_PIN = 6;
HardwareSerial mp3Serial(1); // UART1
DFRobotDFPlayerMini mp3;
const int M1_P = 39, M1_L = 37, M1_R = 38, M2_R = 26, M2_L = 21, M2_P = 33;
const int M3_P = 34, M3_L = 35, M3_R = 36, M4_R = 19, M4_L = 20, M4_P = 18;
// const int sensor_pins[] = {13, 14, 15, 16, 17};
const int sensor_pins[] = {16, 17, 15, 14, 13};
Motor m1(M1_L, M1_R, M1_P);
Motor m2(M2_L, M2_R, M2_P);
Motor m3(M3_L, M3_R, M3_P);
Motor m4(M4_L, M4_R, M4_P);
MotorPowerAverager motorPowerAverager(50);
Sensor sensor((int *)sensor_pins, 5, 2750);
StateManager state_manager;
TimeoutManager timeout_manager;

const int RUN_SP = 100;
const int HALF_STOP_SP = 50;
const int STOP_SP = -20;

// -255 ~ 255
void run_motor(int left_speed, int right_speed)
{
  // m1.run(left_speed);
  // m2.run(left_speed);
  // m3.run(right_speed);
  // m4.run(right_speed);
  motorPowerAverager.addPower(left_speed, right_speed);
}

void stop_motor()
{
  // m1.stop();
  // m2.stop();
  // m3.stop();
  // m4.stop();
  motorPowerAverager.addPower(0, 0);
}

void _run_motor()
{
  m1.run(motorPowerAverager.getL());
  m2.run(motorPowerAverager.getL());
  m3.run(motorPowerAverager.getR());
  m4.run(motorPowerAverager.getR());
}

StateMapEntry state_map[] = {
    // center (like) : 0
    {0b00100, 0},
    {0b01110, 0},
    {0b01010, 0},
    {0b01100, 0},
    {0b00110, 0},
    {0b11110, 0},
    {0b01111, 0},
    {0b11111, 0},
    {0b01101, 0},
    {0b10110, 0},
    // Left (like) : 1
    //{0b10000, 1},
    //{0b11000, 1},
    {0b11100, 1},
    {0b11110, 1},
    // Right (like) : 2
    //{0b00001, 2},
    //{0b00011, 2},
    {0b00111, 2},
    {0b01111, 2},
    // all or none : 3
    {0b00000, 3}};

const int C_detection_threshold = 20;

void setup()
{
  delay(2000);

  Serial.begin(115200);
  Serial.println("Starting...");

  pinMode(6, OUTPUT);
  m1.setup();
  m2.setup();
  m3.setup();
  m4.setup();
  sensor.setup();

  state_manager.setStateMap(state_map, sizeof(state_map) / sizeof(StateMapEntry));
}

void loop()
{
  sensor.read();

  digitalWrite(LED_PIN, LOW);

  // calc state
  if (state_manager.updateState(sensor.state()))
  {
    // state changed
    Serial.print("State changed: ");
    Serial.print(state_manager.getPrevState());
    Serial.print(" -> ");
    Serial.print(state_manager.getCurrentState());
    Serial.print(" | Count: ");
    Serial.println(state_manager.getPrevStateCount());
    if (
        state_manager.getPrevState() == 2 &&
        (state_manager.getCurrentState() == 0 || state_manager.getCurrentState() == -1) &&
        state_manager.getPrevStateCount() >= C_detection_threshold &&
        timeout_manager.get(0) == false)
    {
      timeout_manager.set(0, 10000);
      Serial.print("C detected! ");
      Serial.print("State count: ");
      Serial.println(state_manager.getPrevStateCount());
      digitalWrite(LED_PIN, HIGH);
      motorPowerAverager.ForceSet(STOP_SP, RUN_SP);
      _run_motor();
      delay(1000);
    }
  }
  if (
      state_manager.getCurrentState() == 3 &&
      state_manager.getCurrentStateCount() >= C_detection_threshold * 10 &&
      timeout_manager.get(1) == false)
  {
    timeout_manager.set(1, 3000);
    Serial.print("C return detected! ");
    Serial.print("State count: ");
    Serial.println(state_manager.getCurrentStateCount());
    digitalWrite(LED_PIN, HIGH);
    motorPowerAverager.ForceSet(-RUN_SP, RUN_SP);
    _run_motor();
    delay(1000);
  }

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
  case 0b10100:
    run_motor(RUN_SP, STOP_SP);
    break;
  case 0b10010:
    run_motor(RUN_SP, STOP_SP);
    break;
  case 0b10001:
    run_motor(RUN_SP, STOP_SP);
    break;
  case 0b01010:
    run_motor(RUN_SP, RUN_SP);
    break;
  case 0b01001:
    run_motor(HALF_STOP_SP, RUN_SP);
    break;
  case 0b00101:
    run_motor(STOP_SP, RUN_SP);
    break;
  case 0b11010:
    run_motor(RUN_SP, STOP_SP);
    break;
  case 0b11001:
    run_motor(RUN_SP, STOP_SP);
    break;
  case 0b10110:
    run_motor(RUN_SP, STOP_SP);
    break;
  case 0b10011:
    run_motor(RUN_SP, STOP_SP);
    break;
  case 0b01101:
    run_motor(HALF_STOP_SP, RUN_SP);
    break;
  case 0b01011:
    run_motor(HALF_STOP_SP, RUN_SP);
    break;
  case 0b00000:
    // do nothing
    break;
  default:
    break;
  }
  // Apply averaged motor powers
  _run_motor();
}
