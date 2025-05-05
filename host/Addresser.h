#ifndef ADDRESSER_H
#define ADDRESSER_H

#define BROADCAST_ADDRESS 0xff
#define ADDRESS_HEADER_LENGTH 4

bool address(uint8_t* buf, uint8_t* tbuf, uint8_t dataOffset, uint8_t bufLength, uint8_t to, uint8_t from){
  memcpy(tbuf+ADDRESS_HEADER_LENGTH+dataOffset,buf,bufLength);

  uint8_t checksum = 0;
  for(int i = 0; i < bufLength; i++) checksum += buf[i];

  tbuf[0] = from;
  tbuf[1] = to;
  tbuf[2] = checksum;
  tbuf[3] = 0;
  return (bufLength + ADDRESS_HEADER_LENGTH <= RH_RF95_MAX_MESSAGE_LEN);
}

#endif