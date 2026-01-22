#ifndef _timeout_manager_h
#define _timeout_manager_h

#include <Arduino.h>

#define MAX_TIMEOUTS 16

class TimeoutManager
{
private:
    unsigned long timeout_times[MAX_TIMEOUTS];
    bool is_set[MAX_TIMEOUTS];

public:
    TimeoutManager();
    void set(int id, unsigned long time_ms);
    bool get(int id);
    void clear(int id);
    bool getCurrentState(int id);
};

TimeoutManager::TimeoutManager()
{
    for (int i = 0; i < MAX_TIMEOUTS; i++)
    {
        timeout_times[i] = 0;
        is_set[i] = false;
    }
}

void TimeoutManager::set(int id, unsigned long time_ms)
{
    if (id < 0 || id >= MAX_TIMEOUTS)
        return;
    timeout_times[id] = millis() + time_ms;
    is_set[id] = true;
}

bool TimeoutManager::get(int id)
{
    if (id < 0 || id >= MAX_TIMEOUTS)
        return false;
    if (!is_set[id])
        return false;

    // オーバーフロー対応の時間比較（差分計算）
    unsigned long current_time = millis();
    if ((long)(timeout_times[id] - current_time) > 0)
    {
        return true;
    }
    else
    {
        is_set[id] = false;
        return false;
    }
}

void TimeoutManager::clear(int id)
{
    if (id < 0 || id >= MAX_TIMEOUTS)
        return;
    is_set[id] = false;
}

bool TimeoutManager::getCurrentState(int id)
{
    if (id < 0 || id >= MAX_TIMEOUTS)
        return false;
    if (!is_set[id])
        return false;

    // オーバーフロー対応の時間比較（差分計算）
    unsigned long current_time = millis();
    return (long)(timeout_times[id] - current_time) > 0;
}

#endif
