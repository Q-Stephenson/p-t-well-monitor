#include "reading.h"

uint8_t Reading::serialize(uint8_t* buf, uint8_t subcmd){
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

void Reading::uploadR2(uint8_t* buf, uint8_t subcmd){
  uint8_t pt = 0;
  if(subcmd & 1){
    memcpy((uint8_t*) &temp,buf+pt,4);
  }
  if((subcmd>>1) & 1){
    pt+=4;
    memcpy((uint8_t*) &SpC,buf+pt,4);
  }
  if((subcmd>>2) & 1){
    pt+=4;
    memcpy((uint8_t*) &pH,buf+pt,4);
  }
  if((subcmd>>3) & 1){
    pt+=4;
    memcpy((uint8_t*) &stepDist,buf+pt,4);
  }
  tlm = millis();
}

String Reading::getString(){
  return "Temp: " + String(temp) + "\nSpC: " + String(SpC) + "\npH: " + String(pH) + "\nStepDist: " + String(stepDist) + "\nTime Since Last Measurement: " + String(tlm - millis()) + "\n";
}