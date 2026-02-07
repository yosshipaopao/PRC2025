# CROSS動作 設計案

## 目的
`crossCount`が増えるたびに、LED/サーボ/音の状態を自動で切り替える仕組みを追加する。

## 方針
- `crossCount`の変化検知で動作を発火する（CROSS検知時にのみ増える）。
- `crossCount`ごとの動作定義をリスト（テーブル）で管理する。
- CROSS検知の処理は以下の流れにする。
  1. `crossCount++`
  2. `applyCrossAction(crossCount)` を呼び出す

## 追加するデータ構造

### `CrossAction` 構造体
- `ledBlinkCount` (int)
- `ledBlinkIntervalMs` (int)
- `servo1` (int)
- `servo2` (int)
- `servo3` (int)
- `servo4` (int)
- `servoHoldMs` (int)
- `soundIndex` (int)

### アクションリスト
`crossCount`を1始まりで参照し、範囲外は最後の定義を繰り返す。

例:

| crossCount | ledBlinkCount | ledBlinkIntervalMs | servo1 | servo2 | servo3 | servo4 | servoHoldMs | soundIndex |
|-----------:|--------------:|-------------------:|-------:|-------:|-------:|-------:|------------:|-----------:|
| 1 | 3 | 100 | 180 | 180 | 180 | 180 | 1000 | 1 |
| 2 | 1 | 200 | 90  | 90  | 90  | 90  | 500  | 2 |
| 3 | 2 | 150 | 180 | 0   | 180 | 0   | 800  | 3 |

## 実装イメージ

### `applyCrossAction(int count)`
- `CrossAction`配列から対象を取得
- 音を再生（`mp3.play(soundIndex)`）
- LED点滅（回数・間隔は定義に従う）
- サーボ角度を設定→保持→初期位置へ戻す

### `handleCrossByCount()` 変更点
- 既存の`handleCrossByCount()`は削除/簡素化
- CROSS検知時は `applyCrossAction(crossCount)` のみ呼び出す

## 追加・変更予定ファイル
- `src/main.cpp`：
  - `CrossAction`定義
  - `crossActions[]`配列
  - `applyCrossAction()`追加
  - CROSS処理を `applyCrossAction()` 呼び出しに変更

## 未決定事項（承認後に実装）
- `CrossAction`の具体的なパラメータ
- `crossCount`上限時の挙動（最後の定義を繰り返す／何もしない）

---

この設計でよければ「OK」と返信してください。問題があれば修正点を教えてください。
