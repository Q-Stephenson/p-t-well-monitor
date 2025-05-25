#ifndef MOTOR_H
#define MOTOR_H
#include <stdio.h>
#include <hardware/gpio.h>
#include <Arduino.h>
#include "PID.h"

#define LIMIT_PIN 15
#define MAX_SPIN_COUNT 4096 /*TODO: FIX*/
#define RETRACT_SPEED 230

#define RADIUS 0.25 /*In Meters*/

namespace motor{
    extern long steps; // Equals the encoderPos when the motor is at the limit switch, 0 of the system

    long getSteps(); // Gets the number of steps

    void motorThread();
    void init(); // Initializes the motor
    void step(int num); // Steps num steps
    void step(double radians); // Steps radians
    void step(double meters, double rad); // steps the number of meters provided, given the radius of the wheel, presumes no slipping
    void goToStep(int num);
    void reset(); // Resets the motor, TODO: uses the limit switch to zero
    void reclaimPins();
    void vibrate(uint32_t ms); // SCARY

    bool isPressed();
};

namespace motor_utils{
    float stepsToMeters(int steps); // Converts Steps to Meters
    int metersToSteps(float dist); // Converts Meters to Steps
};

#endif