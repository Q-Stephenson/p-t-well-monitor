#ifndef MOTOR_H
#define MOTOR_H
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>

class Motor{
private:
    const static uint IN1 = 0;
    const static uint IN2 = 2;
    const static uint IN3 = 3;
    const static uint IN4 = 4;
    int steps = 0;

public:
    Motor(); // Initializes the motor with steps=0;
    Motor(int stepCount); // Initializes the number of steps as the stepCount

    int getSteps(){return steps;} // Gets the number of steps

    void init(); // Initializes the motor
    void step(); // Steps forward one step
    void backstep(); // Backstep one step
    void stop(); // Stops the motors activation
    void step(int num); // Steps num steps
    void step(double radians); // Steps radians
    void step(double meters, double rad); // steps the number of meters provided, given the radius of the wheel, presumes no slipping
    void reset(); // Resets the motor, TODO: uses the limit switch to zero
};

#endif