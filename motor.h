#ifndef MOTOR_H
#define MOTOR_H
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>

namespace motor{
    const uint IN1 = 0;
    const uint IN2 = 2;
    const uint IN3 = 3;
    const uint IN4 = 4;
    void init();
    void step();
    void step(int num);
    void step(double radians);
    void step(double meters, double rad);
}

#endif