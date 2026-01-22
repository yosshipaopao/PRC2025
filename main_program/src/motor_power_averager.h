#ifndef _motor_power_averager_h
#define _motor_power_averager_h

#include <Arduino.h>

class MotorPowerAverager
{
private:
    size_t size;
    int *left_powers;
    int *right_powers;
    size_t index;
    size_t count;
    int left_sum;
    int right_sum;

public:
    MotorPowerAverager(size_t size);
    void addPower(int l, int r);
    int getL();
    int getR();
    void forceClear()
    {
        index = 0;
        count = 0;
        left_sum = 0;
        right_sum = 0;
    }
    void ForceSet(int l, int r)
    {
        forceClear();
        for (size_t i = 0; i < size; i++)
        {
            addPower(l, r);
        }
    }
};

MotorPowerAverager::MotorPowerAverager(size_t size)
{
    this->size = size;
    left_powers = new int[size];
    right_powers = new int[size];
    // Initialize arrays to 0
    for (size_t i = 0; i < size; i++)
    {
        left_powers[i] = 0;
        right_powers[i] = 0;
    }
    index = 0;
    count = 0;
    left_sum = 0;
    right_sum = 0;
}

void MotorPowerAverager::addPower(int l, int r)
{
    if (count < size)
    {
        count++;
    }
    else
    {
        left_sum -= left_powers[index];
        right_sum -= right_powers[index];
    }
    left_powers[index] = l;
    right_powers[index] = r;
    left_sum += l;
    right_sum += r;
    index = (index + 1) % size;
}
int MotorPowerAverager::getL()
{
    if (count == 0)
        return 0;
    int weighted_sum = 0;
    int weight_sum = 0;
    for (size_t i = 0; i < count; i++)
    {
        // Calculate position correctly: go backwards from current index
        size_t pos = (index + size - count + i) % size;
        int weight = i + 1;
        weighted_sum += left_powers[pos] * weight;
        weight_sum += weight;
    }
    return weighted_sum / weight_sum;
}
int MotorPowerAverager::getR()
{
    if (count == 0)
        return 0;
    int weighted_sum = 0;
    int weight_sum = 0;
    for (size_t i = 0; i < count; i++)
    {
        // Calculate position correctly: go backwards from current index
        size_t pos = (index + size - count + i) % size;
        int weight = i + 1;
        weighted_sum += right_powers[pos] * weight;
        weight_sum += weight;
    }
    return weighted_sum / weight_sum;
}
#endif