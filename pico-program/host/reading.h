#ifndef READING_H
#define READING_H

#include <cstdint>
#include <cstring>
#include "Arduino.h"

class Reading{
  private:
    uint32_t stepDist = 0; /*Number of steps from zeroed motor to the bottom of the well*/
    float temp = 0; /*Temperature in Celcius*/
    float pH = 0;
    float SpC = 0;
    uint32_t tlm = 0; /*Time of last measurement, measured using Arduino millis()*/

  public:
    Reading(){}

    Reading(uint32_t dist, float _temp, float _pH, float _SpC, uint32_t _tlm) : stepDist(dist), temp(_temp), pH(_pH), SpC(_SpC), tlm(_tlm) {}

    void uploadR2(uint8_t* buf, uint8_t subcmd);
    uint8_t serialize(uint8_t* buf,uint8_t subcmd); //Returns: Length of Buf, Buf size: 4(number of measurements requested)
    String getString();
};

#endif