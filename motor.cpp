#include "motor.h"

Motor::Motor(int stepCounts){
    steps = stepCounts;
}

Motor::Motor(){
    Motor(0);
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

void Motor::stop(){
    gpio_put(IN1, 0);
    gpio_put(IN2, 0);
    gpio_put(IN3, 0);
    gpio_put(IN4, 0);
}

void Motor::init(){
    // Initialize the gpio library
    gpio_init(IN1);
    gpio_init(IN2);
    gpio_init(IN3);
    gpio_init(IN4);

    // Set the mode of the pin to output
    gpio_set_dir(IN1, GPIO_OUT);
    gpio_set_dir(IN2, GPIO_OUT);
    gpio_set_dir(IN3, GPIO_OUT);
    gpio_set_dir(IN4, GPIO_OUT);

    // Set the initial state of the pins to LOW
    gpio_put(IN1, 0);
    gpio_put(IN2, 0);
    gpio_put(IN3, 0);
    gpio_put(IN4, 0);    
}

void Motor::step(){
    // Step 4
    gpio_put(IN1, 0);
    gpio_put(IN2, 0);
    gpio_put(IN3, 1);
    gpio_put(IN4, 1);
    sleep_ms(3);

    // Step 3
    gpio_put(IN1, 0);
    gpio_put(IN2, 1);
    gpio_put(IN3, 1);
    gpio_put(IN4, 0);
    sleep_ms(3);

    // Step 2
    gpio_put(IN1, 1);
    gpio_put(IN2, 1);
    gpio_put(IN3, 0);
    gpio_put(IN4, 0);
    sleep_ms(3);

    // Step 1
    gpio_put(IN1, 1);
    gpio_put(IN2, 0);
    gpio_put(IN3, 0);
    gpio_put(IN4, 1);
    sleep_ms(3);

    steps++;
}

void Motor::backstep(){        
    // Step 1
    
    gpio_put(IN1, 1);
    gpio_put(IN2, 0);
    gpio_put(IN3, 0);
    gpio_put(IN4, 1);
    sleep_ms(3); 

    // Step 2
    gpio_put(IN1, 1);
    gpio_put(IN2, 1);
    gpio_put(IN3, 0);
    gpio_put(IN4, 0);
    sleep_ms(3);
    
    // Step 3
    gpio_put(IN1, 0);
    gpio_put(IN2, 1);
    gpio_put(IN3, 1);
    gpio_put(IN4, 0);
    sleep_ms(3);
    
    // Step 4
    gpio_put(IN1, 0);
    gpio_put(IN2, 0);
    gpio_put(IN3, 1);
    gpio_put(IN4, 1);
    sleep_ms(3);

    steps--;
}