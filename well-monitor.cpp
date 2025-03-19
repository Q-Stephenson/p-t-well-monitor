#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <string>
#include "motor.h"

// Stepper motor connection pins


int main()
{
    stdio_init_all();

    save::read();

    Motor motor(save::sysData.steps);
    motor.init();
    printf("%s", std::to_string(motor.getSteps()).c_str());

    motor.step(-motor.getSteps());

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