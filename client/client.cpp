// Client side of the system

#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <string>
#include "motor.h"

// Stepper motor connection pins

struct clientData {
    int id;
};

int main()
{
    /*
    * Defines the system
    */
    stdio_init_all();

    Motor motor; // Correctly instantiate the Motor object
    motor.init();

    /*
    * Zeros the motor
    */
    motor.reset();

    /*
    * Connects to host system
    */
    // CONNECT TO HOST SYSTEM ("Data connection")

    /*
    * Wait for Host Request
    */

    /*
    * Drop wire until contact with water
    */

    /*
    * Gather measurements
    */

    /*
    * Send data to host
    */

    return 0;
}