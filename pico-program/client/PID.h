#ifndef PID_H
#define PID_H

#include <Arduino.h>
#include <PID_v1.h>

#define Kp 0.45
#define Ki 0.002
#define Kd 0.07

#define encoderPinA 12
#define encoderPinB 13
#define PINA 0
#define PINB 1
#define PINEN 9
#define MIN_POWER 190

#define TURN_OFF_DIST 40

namespace pid{
  extern bool runMotor;

  extern volatile long encoderPos;
  extern double setpoint;
  extern double input, output;

  extern PID myPID;

  void reclaimPins();
  void init();
  bool isThere();
  void goTo(long posTo);
  void step(long count);
  void loopProcess();
  void driveMotor(double power);
  void updateEncoder();
}

#endif