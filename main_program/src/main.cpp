#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include "debug.h"
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
const int sensor_pins[] = {13, 14, 15, 16, 17, 12};
Motor m1(M1_L, M1_R, M1_P);
Motor m2(M2_L, M2_R, M2_P);
Motor m3(M3_L, M3_R, M3_P);
Motor m4(M4_L, M4_R, M4_P);
MotorPowerAverager motorPowerAverager(50);
Sensor sensor((int *)sensor_pins, 6, 2000);
StateManager state_manager;
TimeoutManager timeout_manager;

const int RUN_SP = 130;
const int HALF_STOP_SP = 70;
const int STOP_SP = 0;
const unsigned long LOOP_INTERVAL = 5; // ms
int cross_count = 0;
unsigned long nextLoopTime = 0;

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
    {0b01101, 0},
    {0b10110, 0},
    // Left (like) : 1
    {0b110000, 1},
    {0b111000, 1},
    {0b111100, 1},
    {0b111110, 1},
    // Right (like) : 2
    //{0b00001, 2},
    //{0b00011, 2},
    {0b00111, 2},
    {0b01111, 2}, // 右寄り
    // none : 3
    {0b00000, 3},
    // all : 4
    {0b11111, 4},
    {0b111111, 4},
};

const int C_detection_threshold = 10;

void setup()
{
  delay(2000);

  Debug::begin(115200);
  Debug::println("Starting...");

  pinMode(6, OUTPUT);
  m1.setup();
  m2.setup();
  m3.setup();
  m4.setup();
  sensor.setup();

  state_manager.setStateMap(state_map, sizeof(state_map) / sizeof(StateMapEntry));

  nextLoopTime = millis();
}

void loop()
{
  yield(); // WDTをリセット

  // 次の実行タイミングまで待機
  unsigned long currentTime = millis();
  if (currentTime < nextLoopTime)
  {
    unsigned long waitTime = nextLoopTime - currentTime;
    if (waitTime > 0 && waitTime < 1000) // 異常値チェック
    {
      delay(waitTime);
    }
  }
  // 処理が遅れた場合は次のタイミングを現在時刻基準にリセット
  if ((long)(currentTime - nextLoopTime) > 100)
  {
    nextLoopTime = currentTime;
  }
  nextLoopTime += LOOP_INTERVAL;

  sensor.read();
  int current_sensor_state = sensor.state(); // キャッシュして複数回の呼び出しを防ぐ

  // calc state
  if (state_manager.updateState(current_sensor_state))
  {
    // state changed
    Debug::print("State changed: ");
    Debug::print(state_manager.getPrevState());
    Debug::print(" -> ");
    Debug::print(state_manager.getCurrentState());
    Debug::print(" | Count: ");
    Debug::println(state_manager.getPrevStateCount());

    if (
        state_manager.getPrevState() == 1 &&
        (state_manager.getCurrentState() == 0 || state_manager.getCurrentState() == -1) &&
        state_manager.getPrevStateCount() >= C_detection_threshold &&
        timeout_manager.get(0) == false)
    {
      Debug::print("C detected! ");
      Debug::print("State count: ");
      Debug::println(state_manager.getPrevStateCount());
      motorPowerAverager.ForceSet(STOP_SP, RUN_SP);
      _run_motor();
      delay(2000);
      timeout_manager.set(0, 5000);
    }
    if (
        state_manager.getPrevState() == 4 &&
        (state_manager.getCurrentState() == 0 || state_manager.getCurrentState() == -1) &&
        state_manager.getPrevStateCount() >= C_detection_threshold &&
        timeout_manager.get(3) == false)
    {
      timeout_manager.set(3, 3000);
      Debug::print("Cross detected! ");
      cross_count++;
      Debug::print("Count: ");
      Debug::println(cross_count);
    }
  }
  if (
      state_manager.getCurrentState() == 3 &&
      state_manager.getCurrentStateCount() >= C_detection_threshold * 10 &&
      timeout_manager.get(1) == false &&
      timeout_manager.get(0) == false)
  {
    Debug::print("C return detected! ");
    Debug::print("State count: ");
    Debug::println(state_manager.getCurrentStateCount());
    motorPowerAverager.ForceSet(-RUN_SP, RUN_SP);
    _run_motor();
    delay(2000);
    timeout_manager.set(1, 5000);
  }

  // Line Tracer Logic
  switch (current_sensor_state)
  {
  case 0b11111:
  case 0b01110:
  case 0b00100:
  case 0b01010:
  case 0b01101:
  case 0b10110:
  case 0b11110:
    run_motor(RUN_SP, RUN_SP);
    break;
  case 0b01100:
  case 0b11100:
    run_motor(HALF_STOP_SP, RUN_SP);
    break;
  case 0b00110:
  case 0b00111:
    run_motor(RUN_SP, HALF_STOP_SP);
    break;
  case 0b01000:
  case 0b11000:
    run_motor(STOP_SP, RUN_SP);
    break;
  case 0b00010:
  case 0b00011:
    run_motor(RUN_SP, STOP_SP);
    break;
  case 0b10000:
    run_motor(-RUN_SP, RUN_SP);
    break;
  case 0b00001:
    run_motor(RUN_SP, -RUN_SP);
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
