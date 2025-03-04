#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include "motor.h"
#include "save.h"

// Stepper motor connection pins


int main()
{
    Motor motor;
    motor.init();

    // Loop indefinitely
    while (1)
    {
        motor.step(6.28);
        motor.stop();
        sleep_ms(1000);
        motor.step(-6.28);
    }

    return 0;
}