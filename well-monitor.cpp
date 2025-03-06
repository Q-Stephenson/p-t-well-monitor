#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include "motor.h"
#include "save.h"

// Stepper motor connection pins


int main()
{
    save::read();

    Motor motor(save::sysData.steps);
    motor.init();

    motor.step(-motor.getSteps());

    // Loop indefinitely
    while (1)
    {
        motor.step(6.28);
        motor.stop();
        sleep_ms(1000);
        motor.step(-6.28);
        save::sysData.steps = motor.getSteps();
        save::save();
    }

    return 0;
}