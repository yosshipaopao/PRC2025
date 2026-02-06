#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// ボタンピン定義（プルアップ）
#define BUTTON_PIN_1 9
#define BUTTON_PIN_2 10
#define BUTTON_PIN_3 11

// LED定義
#define LED_PIN 18 // BUILTIN_LED

// NeoPixel設定（1個のLED）
Adafruit_NeoPixel pixels(1, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  // シリアル通信初期化
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32-S2 NeoPixel Button Control ===");

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
  Serial.println("======================================\n");
}

void loop()
{
  // ボタンの状態を読み込む（プルアップのため、押下時は LOW）
  int button1 = digitalRead(BUTTON_PIN_1);
  int button2 = digitalRead(BUTTON_PIN_2);
  int button3 = digitalRead(BUTTON_PIN_3);

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