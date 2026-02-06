#ifndef TIMEOUT_MANAGER_H
#define TIMEOUT_MANAGER_H

#include <Arduino.h>

/**
 * タイムアウト管理クラス
 * 複数のイベントの再発動を防ぐため、待機時間を管理
 */
class TimeoutManager
{
private:
    static const int MAX_TIMEOUTS = 16;        // 最大タイムアウト数
    unsigned long timeout_times[MAX_TIMEOUTS]; // タイムアウト時刻
    bool is_set[MAX_TIMEOUTS];                 // タイムアウト設定フラグ

public:
    /**
     * コンストラクタ
     */
    TimeoutManager()
    {
        for (int i = 0; i < MAX_TIMEOUTS; i++)
        {
            timeout_times[i] = 0;
            is_set[i] = false;
        }
    }

    /**
     * タイムアウトを設定
     * @param id タイムアウト ID（0～15）
     * @param ms タイムアウト時間（ミリ秒）
     */
    void set(int id, unsigned long ms)
    {
        if (id < 0 || id >= MAX_TIMEOUTS)
            return;

        timeout_times[id] = millis() + ms;
        is_set[id] = true;
    }

    /**
     * タイムアウトが経過したか判定
     * @param id タイムアウト ID（0～15）
     * @return false: タイムアウト中（まだ使用不可）, true: 使用可能
     */
    bool get(int id)
    {
        if (id < 0 || id >= MAX_TIMEOUTS)
            return true;

        if (!is_set[id])
        {
            return true; // 設定されていない場合は使用可能
        }

        unsigned long current_time = millis();

        // オーバーフロー対応（差分で比較）
        if ((long)(current_time - timeout_times[id]) >= 0)
        {
            // タイムアウト経過
            is_set[id] = false;
            return true;
        }

        // まだタイムアウト中
        return false;
    }

    /**
     * タイムアウトをクリア
     * @param id タイムアウト ID（0～15）
     */
    void clear(int id)
    {
        if (id < 0 || id >= MAX_TIMEOUTS)
            return;

        is_set[id] = false;
        timeout_times[id] = 0;
    }

    /**
     * 全タイムアウトをクリア
     */
    void clearAll()
    {
        for (int i = 0; i < MAX_TIMEOUTS; i++)
        {
            is_set[i] = false;
            timeout_times[i] = 0;
        }
    }
};

#endif // TIMEOUT_MANAGER_H
