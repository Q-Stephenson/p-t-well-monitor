#include "PID.h"

namespace pid{
  bool runMotor = 1;

  volatile long encoderPos = 0;
  double setpoint = 0;
  double input = 0, output = 0;

  PID myPID(&input,&output,&setpoint, Kp, Ki, Kd, DIRECT);

  void init() {
    Serial.begin(115200);
    pinMode(encoderPinA, INPUT);
    pinMode(encoderPinB, INPUT);
    pinMode(PINA, OUTPUT);
    pinMode(PINB, OUTPUT);
    pinMode(PINEN, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
    attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);

    myPID.SetMode(AUTOMATIC);
    myPID.SetOutputLimits(-255, 255);  // PWM range for L298N
  }

  void reclaimPins(){
    pinMode(encoderPinA, INPUT);
    pinMode(encoderPinB, INPUT);
    pinMode(PINA, OUTPUT);
    pinMode(PINB, OUTPUT);
    pinMode(PINEN, OUTPUT);
  }

  bool isThere(){
    return abs(output) < 30;
  }

  void goTo(long posTo){
    setpoint = posTo;
    do{
      delay(5);
    }while(!isThere());
    digitalWrite(PINA, LOW);
    digitalWrite(PINB, LOW);
  }

  void step(long count){
    goTo(encoderPos + count);
  }

  void loopProcess() {
    input = encoderPos;
    myPID.Compute();
    driveMotor(output);
  }

  void driveMotor(double power) {
    power = constrain(power, -255, 255);
    analogWrite(PINEN, max(abs(power),MIN_POWER));

    if(abs(setpoint - encoderPos) < TURN_OFF_DIST && pid::runMotor){
      digitalWrite(PINA,LOW);
      digitalWrite(PINB,LOW);
      sleep_ms(10);
      return;
    }

    if (power > 0) {
      digitalWrite(PINA, HIGH);
      digitalWrite(PINB, LOW);
    } else if (power < 0) {
      digitalWrite(PINA, LOW);
      digitalWrite(PINB, HIGH);
    } else {
      digitalWrite(PINA, LOW);
      digitalWrite(PINB, LOW);
    }
  }

  void updateEncoder() {
    int MSB = digitalRead(encoderPinA);
    int LSB = digitalRead(encoderPinB);

    int encoded = (MSB << 1) | LSB;
    static int lastEncoded = 0;
    int sum = (lastEncoded << 2) | encoded;

    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
      encoderPos++;
    else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
      encoderPos--;

    lastEncoded = encoded;
  }
}