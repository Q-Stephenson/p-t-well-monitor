#include "motor.h"

long motor::steps = 0;

void motor::motorThread(){
    while(1){
        if(pid::runMotor){
            motor::reclaimPins();
            pid::loopProcess();
        }
        sleep_ms(10);
    }
}

void motor::reclaimPins(){
    pinMode(LIMIT_PIN,INPUT);

    pid::reclaimPins();
}

void motor::reset(){
    /*
    * TODO: Code to go to the limit switch to go here
    */

    pid::runMotor = 0;

    while(isPressed()){
        pid::driveMotor(RETRACT_SPEED);
        delay(10);
    }

    pid::runMotor = 1;

    digitalWrite(PINA, LOW);
    digitalWrite(PINB, LOW);

    delay(75);

    steps = pid::encoderPos;
}

bool motor::isPressed(){
    /*Implement Is Pressed Code*/
    if(digitalRead(LIMIT_PIN) == HIGH) return true;
    return false;
}

void motor::step(double distance, double radius){
    step(distance / (6.28 * radius));
}

void motor::step(double radians){
    int stepCount = (int) (radians * 512 / 6.28);
    step(stepCount);
}

void motor::step(int num){
    pid::step(num);
}

void motor::goToStep(int num){
    pid::goTo(num + steps);
}

void motor::init(){
    // Initialize the gpio library
    motor::reclaimPins();

    // Motor reset
    motor::reset();

    pid::init();
}

long motor::getSteps(){return pid::encoderPos;} // Gets the number of steps

namespace motor_utils{
    int metersToSteps(float dist){
        return (int) (512 * dist / (TWO_PI * TWO_PI * RADIUS));
    }
    float stepsToMeters(int steps){
        return steps * (TWO_PI * TWO_PI * RADIUS) / 512.0;
    }
};