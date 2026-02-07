#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>

// ボタンピン定義（プルアップ）
#define BUTTON_PIN_1 9
#define BUTTON_PIN_2 10
#define BUTTON_PIN_3 11

// LED定義
#define LED_PIN 18 // BUILTIN_LED

// サーボピン定義
#define SERVO_PIN_1 43
#define SERVO_PIN_2 44
#define SERVO_PIN_3 45
#define SERVO_PIN_4 46

// NeoPixel設定（1個のLED）
Adafruit_NeoPixel pixels(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// サーボオブジェクト
Servo servo1, servo2, servo3, servo4;

void setup()
{
  // シリアル通信初期化
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32-S2 Servo Control ===");

  // ボタンピンをプルアップ入力に設定
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);

  Serial.println("Button pins (GPIO 10, 11, 12) set to INPUT_PULLUP");

  // NeoPixel初期化
  pixels.begin();
  pixels.setBrightness(255);
  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // 初期色：黒
  pixels.show();

  Serial.println("NeoPixel initialized on GPIO 18");

  // サーボ初期化（43～46ピン）
  servo1.attach(SERVO_PIN_1);
  servo2.attach(SERVO_PIN_2);
  servo3.attach(SERVO_PIN_3);
  servo4.attach(SERVO_PIN_4);

  // 初期位置を設定（90度）
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);

  Serial.println("Servos initialized on GPIO 43, 44, 45, 46");
  Serial.println("======================================\n");
}

void loop()
{
  // ボタンの状態を読み込む（プルアップのため、押下時は LOW）
  int button1 = digitalRead(BUTTON_PIN_1);
  int button2 = digitalRead(BUTTON_PIN_2);
  int button3 = digitalRead(BUTTON_PIN_3);

  // ボタン状態に応じてサーボを制御（すべて0度/180度制御）
  // button1が押下時：servo1を0度に、解放時：180度に
  int angle1 = (button1 == LOW) ? 0 : 180;
  servo1.write(angle1);

  // button2が押下時：servo2を0度に、解放時：180度に
  int angle2 = (button2 == LOW) ? 0 : 180;
  servo2.write(angle2);

  // button3が押下時：servo3を0度に、解放時：180度に
  int angle3 = (button3 == LOW) ? 0 : 180;
  servo3.write(angle3);

  // servo4も0度/180度制御（button1と連動）
  int angle4 = (button1 == LOW) ? 0 : 180;
  servo4.write(angle4);

  // ボタン状態からRGB値を決定
  uint8_t red = (button1 == LOW) ? 255 : 0;
  uint8_t green = (button2 == LOW) ? 255 : 0;
  uint8_t blue = (button3 == LOW) ? 255 : 0;

  // NeoPixel色設定
  pixels.setPixelColor(0, pixels.Color(red, green, blue));
  pixels.show();

  // ボタン状態をシリアル出力（デバッグ用）
  if (button1 == LOW || button2 == LOW || button3 == LOW)
  {
    Serial.printf("Button: [%d, %d, %d] -> RGB(%d, %d, %d)\n",
                  button1, button2, button3, red, green, blue);
  }

  delay(50); // チャタリング対策
}