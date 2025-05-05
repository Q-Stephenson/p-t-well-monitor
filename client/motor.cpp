#include "motor.h"

Motor::Motor(int stepCounts){
    steps = stepCounts;
}

Motor::Motor() : Motor(0){
}

void Motor::reclaimPins(){
    pinMode(LIMIT_PIN,INPUT);

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
}

void Motor::reset(){
    /*
    * TODO: Code to go to the limit switch to go here
    */
    while(isPressed()) backstep();

    steps = 0;
}

bool Motor::isPressed(){
    /*Implement Is Pressed Code*/
    if(digitalRead(LIMIT_PIN) == HIGH) return true;
    return false;
}

void Motor::step(double distance, double radius){
    step(distance / (6.28 * radius));
}

void Motor::step(double radians){
    int stepCount = (int) (radians * 512 / 6.28);
    step(stepCount);
}

void Motor::step(int num){
    if(num > 0){
        for(int i = 0; i < num; i++) step();
    }else{
        for(int i = 0; i < -num; i++) backstep();
    }
}

void Motor::goToStep(int num){
    Motor::step(num - steps);
}

void Motor::stop(){
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}

void Motor::init(){
    // Initialize the gpio library
    Motor::reclaimPins();

    // Motor reset
    Motor::reset();

    // Set the initial state of the pins to LOW
    Motor::stop();    
}

void Motor::step(){
    // Step 4
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, HIGH);
    delay(3);

    // Step 3
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    delay(3);

    // Step 2
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    delay(3);

    // Step 1
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    delay(3);

    steps++;
}

void Motor::backstep(){        
    // Step 1
    
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    delay(3); 

    // Step 2
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    delay(3);
    
    // Step 3
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    delay(3);
    
    // Step 4
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, HIGH);
    delay(3);

    steps--;
}

namespace motor_utils{
    int metersToSteps(float dist){
        return (int) (512 * dist / (TWO_PI * TWO_PI * RADIUS));
    }
    float stepsToMeters(int steps){
        return steps * (TWO_PI * TWO_PI * RADIUS) / 512.0;
    }
};