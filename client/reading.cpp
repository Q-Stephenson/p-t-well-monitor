#include "reading.h"

namespace reading{
  uint32_t stepDist = 0; /*Number of steps from zeroed motor to the bottom of the well*/
  float temp = 0; /*Temperature in Celcius*/
  float pH = 0;
  float SpC = 0;
  uint32_t tlm = 0; /*Time of last measurement, measured using Arduino millis()*/

  uint8_t serialize(uint8_t* buf, uint8_t subcmd){
    uint8_t len = ((subcmd) & 1) + ((subcmd>>1) & 1) + ((subcmd>>2) & 1) + ((subcmd>>3) & 1);
    len *= 4;
    // uint8_t buf[len];
    uint8_t pt = 0;

    if(subcmd & 1)
    {pt=0;
    memcpy(buf+pt,(uint8_t*)&temp,4);}

    if((subcmd>>1) & 1)
    {pt+=4;
    memcpy(buf+pt,(uint8_t*)&SpC,4);}

    if((subcmd>>2) & 1)
    {pt+=4;
    memcpy(buf+pt,(uint8_t*)&pH,4);}

    if((subcmd>>3) & 1)
    {pt+=4;
    memcpy(buf+pt,&stepDist,4);}

    return len;
  }

  int headToWater(Motor* motor){
    if(motor == NULL) return -1; // DEF: -1
    
    (*motor).reset();

    int maxSteps = MAX_SPIN_COUNT;
    int minSteps = 0;
    int avgSteps = (maxSteps + minSteps)>>1;
    while(maxSteps > minSteps){
      (*motor).goToStep(avgSteps);
      delay(100);
      float R2 = readResistance();
      if(R2 > MIN_AIR_RESISTANCE /*IS IN AIR*/){
        Serial.printf("IN AIR: %f\n",R2);
        minSteps = avgSteps;
      }else{
        /*IS IN WATER*/
        Serial.printf("IN WATER: %f\n",R2);
        maxSteps = avgSteps;
      }
      avgSteps = (maxSteps + minSteps)>>1;
    }

    return avgSteps;
  }

  void measure(Motor* motor,uint8_t subcmd){
    if((subcmd>>3) & 1) stepDist = headToWater(motor); else headToWater(motor);
    if((subcmd>>1) & 1){
      delay(100);
      SpC = (PLATE_DIST / (PLATE_AREA * readResistance()));
      delay(100);
    }
    /*TODO: READ TEMP AND PH*/
    tlm = millis();
    (*motor).goToStep(0);
  }

  uint16_t readV(){
    return (uint16_t) analogRead(PIN_VIN);
  }

  float readResistance(){
      uint16_t data[0xff];
      for(int i = 0; i < 0xff; i++){
        data[i] = reading::readV();
      }

      float meanV = 0;
      for(int i = 0; i < 0xff; i++){meanV += (float) data[i];} meanV /= 255.0;

      float VoltageF = (meanV * 3.3) / (1<<10);

      Serial.printf("\n\nMeanV: %.2f\nVoltage: %.2f\n",meanV,VoltageF);

      if(abs(MAX_VOLTAGE - VoltageF) <= 0.0001) return CAPPED_MAX_RESISTANCE;

      float R2 = R1 * (1 / ((MAX_VOLTAGE / VoltageF) - 1));

      return R2;
  }
};