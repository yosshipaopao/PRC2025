#ifndef _state_manager_h
#define _state_manager_h

#include <Arduino.h>

// State map entry: multiple states map to one group_id
struct StateMapEntry
{
    int state;
    int group_id;
};

class StateManager
{
private:
    int state;
    int prev_state;
    int state_count;
    int prev_state_count;
    StateMapEntry *state_map;
    int map_size;

    void printStateBinary(int state)
    {
        for (int i = 4; i >= 0; i--)
        {
            Serial.print((state >> i) & 1);
        }
    }

public:
    StateManager();
    bool updateState(int current_state);
    int getPrevState();
    int getStateCount();
    void resetCount();
    bool query(int state, int cnt);
    int getPrevStateCount();
    void setStateMap(StateMapEntry *map, int size);
    int getCurrentState();
    int getCurrentStateCount() { return state_count; }
    int mapState(int raw_state)
    {
        if (state_map == NULL)
            return raw_state;
        for (int i = 0; i < map_size; i++)
        {
            if (state_map[i].state == raw_state)
                return state_map[i].group_id;
        }
        return -1; // Unregistered state - 前のstateを維持
    }
};

StateManager::StateManager()
{
    state = -1;
    prev_state = -1;
    state_count = 0;
    prev_state_count = 0;
    state_map = NULL;
    map_size = 0;
}

bool StateManager::updateState(int current_state)
{
    int mapped_current = mapState(current_state);

    // If -1 (unregistered state), keep the previous state
    if (mapped_current == -1)
    {
        mapped_current = state;
    }

    if (mapped_current == state)
    {
        state_count++;
        return false;
    }
    else
    {
        if (state_count > 5)
        {
            prev_state = state;
            prev_state_count = state_count;
            state = mapped_current;
            state_count = 0;
            return true;
        }
        state = mapped_current;
        state_count = 0;
        return false;
    }
}

int StateManager::getPrevState()
{
    return prev_state;
}

int StateManager::getStateCount()
{
    return state_count;
}

void StateManager::resetCount()
{
    state_count = 0;
}

bool StateManager::query(int query_state, int cnt)
{
    return (state == query_state) && (state_count >= cnt);
}

int StateManager::getPrevStateCount()
{
    return prev_state_count;
}

void StateManager::setStateMap(StateMapEntry *map, int size)
{
    state_map = map;
    map_size = size;
}

int StateManager::getCurrentState()
{
    return state;
}

#endif
