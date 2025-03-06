#ifndef MOTOR_H
#define MOTOR_H
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include "save.h"

class Motor{
private:
    const static uint IN1 = 0;
    const static uint IN2 = 2;
    const static uint IN3 = 3;
    const static uint IN4 = 4;
    int steps = 0;

public:
    Motor();
    Motor(int stepCount);

    int getSteps(){return steps;}

    void init();
    void step();
    void backstep();
    void stop();
    void step(int num);
    void step(double radians);
    void step(double meters, double rad);
};

#endif