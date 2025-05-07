#ifndef READING_H
#define READING_H

#include <cstdint>
#include <cstring>
#include "motor.h"

#define R1 4700

#define MAX_VOLTAGE 3.3
#define PIN_VIN 26

#define CAPPED_MAX_RESISTANCE 0xffffff
#define MIN_AIR_RESISTANCE 150000

#define PLATE_AREA 1 /*TODO: REPLACE WITH TRUE VALUE*/
#define PLATE_DIST 1 /*TODO: REPLACE WITH TRUE VALUE*/

namespace reading{
  extern uint32_t stepDist; /*Number of steps from zeroed motor to the bottom of the well*/
  extern float temp; /*Temperature in Celcius*/
  extern float pH;
  extern float SpC;
  extern uint32_t tlm; /*Time of last measurement, measured using Arduino millis()*/

  uint16_t readV();
  float readResistance();

  int headToWater();
  void measure(uint8_t subcmd);
  uint8_t serialize(uint8_t* buf,uint8_t subcmd); //Returns: Length of Buf, Buf size: 4(number of measurements requested)
};

#endif