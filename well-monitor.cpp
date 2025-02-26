#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include "motor.h"

// Stepper motor connection pins


int main()
{
    motor::init();

    // Loop indefinitely
    while (1)
    {
        motor::step();
    }

    return 0;
}