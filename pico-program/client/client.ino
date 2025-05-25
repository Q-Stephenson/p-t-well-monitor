// Client side of the system
#include <stdio.h>
#include <hardware/gpio.h>
#include <string>
#include <cstring> // Include for memcpy
#include "RH_RF95.h"
#include "Message.h"
// #include "motor::h"
#include "pico/unique_id.h"
#include "reading.h"
#include "pico/multicore.h"

// Pin configuration for SPI
#define PIN_MISO 16
#define PIN_MOSI 19
#define PIN_SCK 18
#define PIN_CS 17
#define PIN_RST 20
#define PIN_DIO0 21

// UART configuration
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

RH_RF95 rf95(PIN_CS, PIN_DIO0);

uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t from;

uint64_t boardID;

uint8_t myAdr = 0xff;
uint8_t serverAdr = 0xff;

bool inSystem = 0;

uint8_t packetIDGen = 0;

bool recv(uint8_t* tbuf, uint8_t* len,uint8_t* from=NULL /*Would love NULL defaults here*/, uint8_t* to=NULL /*Would love NULL defaults here*/){ // NEEDS TO RUN rf95.recv(tbuf,len) before

  if(buf[3] & 1) return false;

  // Serial.println();
  // for(int i = 0; i < *len; i++) Serial.printf("%d ",tbuf[i]);
  // Serial.println();

  // Verify with Checksum
  uint8_t checksum = tbuf[2];
  uint8_t runningsum = 0;
  for(int i = ADDRESS_HEADER_LENGTH + MESSAGE_HEADER_LENGTH; i < RH_RF95_MAX_MESSAGE_LEN && i < *len; i++) runningsum += tbuf[i];

  uint8_t toAdr = tbuf[1];
  uint8_t fromAdr = tbuf[0];

  if(toAdr == myAdr && toAdr != BROADCAST_ADDRESS){
    if(checksum != runningsum){
      // Leaves and expects a resend
      Serial.println("Failed Checksum");
      Serial.printf("(Calculated) %d != (Provided) %d",runningsum,checksum);
      return false;
    }
    if(toAdr != BROADCAST_ADDRESS){
      if(sendAck(fromAdr,toAdr))
        Serial.println("Sent ACK");
      else
        Serial.println("Failed to Send Ack");
    }
  }

  if(toAdr == BROADCAST_ADDRESS || toAdr == myAdr){
    if(to != NULL) *to = toAdr;
    if(from != NULL) *from = fromAdr;


    handleMSGrcpt(tbuf+ADDRESS_HEADER_LENGTH,*len - ADDRESS_HEADER_LENGTH,toAdr,fromAdr);
  }
  return true;
}

bool send(uint8_t* pbuf /*Payload Buffer*/, uint32_t len /*Payload Length*/, uint8_t to =0xff/*Would love default to 0xff*/, uint8_t from=0xff /*Would love default to 0xff*/, unsigned long timeout=1000 /*Would love default to 1000*/){
  // Single Addressable
  uint8_t thisID = packetIDGen++;
  uint8_t packetLen = (len + PAYLOAD_LEN - 1) / PAYLOAD_LEN;
  uint8_t sbuf[RH_RF95_MAX_MESSAGE_LEN];

  sbuf[ADDRESS_HEADER_LENGTH] = thisID;
  sbuf[ADDRESS_HEADER_LENGTH+1] = 0; // Sequence Num
  sbuf[ADDRESS_HEADER_LENGTH+2] = packetLen;

  for(int i = 0; i < packetLen; i++){
    address(pbuf,sbuf,MESSAGE_HEADER_LENGTH,min(RH_RF95_MAX_MESSAGE_LEN - ADDRESS_HEADER_LENGTH,len), to, from);

    if(len < PAYLOAD_LEN){
      for(int j = len + ADDRESS_HEADER_LENGTH + MESSAGE_HEADER_LENGTH; j < RH_RF95_MAX_MESSAGE_LEN; j++){
        sbuf[j] = 0;
      }
    }

    bool confirmed = false;
    while(!confirmed){


      if(!rf95.send(sbuf,min(RH_RF95_MAX_MESSAGE_LEN,len + ADDRESS_HEADER_LENGTH + MESSAGE_HEADER_LENGTH))){return false;}

      rf95.waitPacketSent();

      delay(50);

      unsigned long startTime = millis();
      if(to == BROADCAST_ADDRESS) break;
      while(!confirmed){
        if(rf95.waitAvailableTimeout(1000)){
          uint8_t len = RH_RF95_MAX_MESSAGE_LEN;
          // Serial.println("Got an Packet, might be an ACK?");
          rf95.recv(buf,&len);
          if((buf[3] & 1) && ((buf[0] == to || buf[1] == 0xff) && (buf[1] == from || buf[1] == 0xff))){
            confirmed = true;
          }else{
            recv(buf,&len);
          }
        }else{
          Serial.println("Failed to get an ACK");
        }
        if(millis() - startTime >= timeout){
          // RESEND MESSAGE
          Serial.printf("Resending message %d", sbuf[2]);
          break;
        }
      }
    }

    pbuf+=PAYLOAD_LEN;
    len-=PAYLOAD_LEN;
    sbuf[ADDRESS_HEADER_LENGTH+1]++;
  }
  return true;
}

void setup()
{
  delay(1000);
  randomSeed(analogRead(A1));
  Serial.begin(115200);
  // while(!Serial) delay(10);
  delay(1000);
  Serial.println("Beginning Setup...");
  
  pinMode(PIN_RST, OUTPUT);
  digitalWrite(PIN_RST,LOW);
  delay(10);
  digitalWrite(PIN_RST, HIGH); // Hold RST high to allow normal operation
  delay(10);

  // Initialize the RF95 module
  if (!rf95.init()) {
    Serial.println("Failed to init rf95");
      return;
  }

  // Set frequency (e.g., 915 MHz)
  if (!rf95.setFrequency(915.0)) {
    Serial.println("Failed to set frequency");
      return;
  }

  // A BoardID generator
  pico_get_unique_board_id((pico_unique_board_id_t*) &boardID);

  motor::init();

  multicore_launch_core1(motor::motorThread);

  Serial.println("Completed Setup");
}

void loop(){

  motor::reclaimPins();
  handleMSGs();

  if(Serial.available()){
    String receivedData = Serial.readString();
    processCMD(receivedData.c_str());
  }

  sleep_ms(1000);

  // Serial.println(reading::readV());
  // return;

  if(!inSystem){
    uint8_t boardIDBuf[8]; memcpy(boardIDBuf,(uint8_t*) &boardID,8);
    {
      uint8_t P0buf[9];
      P0buf[0] = 0x00;
      memcpy(P0buf+1,boardIDBuf,8);
      send(P0buf, 9);
      rf95.waitPacketSent();
      Serial.println("Sent P0");
    }
    Serial.println("Waiting for P1 Receipt");
    if(rf95.waitAvailableTimeout(1000)){
      uint8_t len = sizeof(buf);
      if(rf95.recv(buf, &len)){
        uint8_t type = buf[0]>>4;
        if(type == 1){
          {
            if(memcmp(buf+1,boardIDBuf,8) != 0){return;}
          }
          Serial.println("Receipt: ");
          for(int i = 0; i < len; i++) Serial.printf("%d ", buf[i]);
          Serial.println();
          myAdr = buf[9];
          serverAdr = buf[10];
          inSystem = 1;
          Serial.printf("My Address is: %d\nServer Address is: %d\n",myAdr,serverAdr);
        }
      }
    }else{
      Serial.println("Failed to recieve P1");
    }
    return;
  }
  // Check if a message is available
  if (rf95.available()) {
      Serial.println("Received a packet");
      // Receive the message
      uint8_t len = sizeof(buf);
      rf95.recv(buf,&len);
      recv(buf,&len);
  }
}

void handleP2(uint8_t* pbuf, uint8_t len,uint8_t from){
  uint8_t cmd = pbuf[0] & 0x0f;
  Serial.printf("-C%d",cmd);
  switch(cmd){
    case 0: motor::reset();
    sendR0(pbuf,from);
    break;

    case 1: Serial.println("Cannot MOVE");
    sendR0(pbuf,from);
    break;

    case 2: 
    reading::measure(0xf);
    sendR0(pbuf,from); 
    break;

    case 4: reading::measure(0xf); /*Collect All Readings*/
    case 3: /*Send Readings as Stored, Send R2*/ 
    sendR2(pbuf,from);
    break;

    case 5: 
    handleC5s(pbuf,len,from);
    sendR0(pbuf,from);
    break;

    case 6: 
    /*Get Client Info*/ 
    break;

    case 7: /*Ping*/
    sendR0(pbuf,from);
    break;

    case 8: 
    handleC8s(pbuf,len);
    sendR0(pbuf,from);
    break;
  }
}

bool sendR0(uint8_t* pbuf, uint8_t from){
  uint8_t cmd = pbuf[0] & 0x0f;
  uint8_t subcmd = 0;
  switch(cmd){
    case 5: case 8: subcmd = (pbuf[6]>>4); break;
  }
  {
      uint8_t rbuf[8];
      rbuf[0] = 0x30;
      memcpy(rbuf+1,pbuf+1,4);
      rbuf[5] = pbuf[5];
      rbuf[6] = (cmd<<4) | subcmd;
      rbuf[7] = 1;
      return send(rbuf, 8, from, myAdr);
    } // R0
}

bool sendR2(uint8_t* pbuf, uint8_t from){
  uint8_t cmd = pbuf[0] & 0x0f;
  uint8_t subcmd = 0;
  switch(cmd){
    case 5: case 8: subcmd = (pbuf[6]>>4); break;
  }
  {
      uint8_t rbuf[24];
      rbuf[0] = 0x32;
      memcpy(rbuf+1,pbuf+1,4);
      rbuf[5] = pbuf[5];
      rbuf[6] = (cmd<<4) | subcmd;
      rbuf[7] = 1;
      reading::serialize(rbuf+8, 0xf);
      return send(rbuf, 24, from, myAdr);
    } // R2
}

void handleMSGrcpt(uint8_t* pbuf, uint8_t len, uint8_t to, uint8_t from){
  uint8_t id = pbuf[0];
  Message* msg = getByID(id);
  if(msg == NULL){
    for(int i = 0; i < MAX_MESSAGE_COUNT; i++){
      if(messages[i] == NULL){
        messages[i] = new Message(pbuf,len);

        (*messages[i]).setAdr(from,to);

        return;
      }
    }
    // OVERLOAD's MSG COUNT
    return;
  }
  msg->addP4(pbuf, len);
  return;
}

void handleC5s(uint8_t* pbuf, uint8_t len, uint8_t from){
  uint8_t subcmd = (pbuf[6]>>4);
  reading::measure(subcmd);
  sendR0(pbuf,from);
}

void handleC8s(uint8_t* pbuf, uint8_t len){
  // TODO: Implement Responses
  uint8_t subcmd = pbuf[6]>>4;
  Serial.printf("-M%d",subcmd);
  int32_t arg1;
  memcpy((uint8_t*) &arg1, pbuf+7, 4);
  Serial.printf("(%d)",arg1);
  switch(subcmd){
    case 0: Serial.println("UNIMPLEMENTED STOP"); break;
    case 1: motor::step((int) arg1); break;
    case 2: motor::reset(); break;
  }
  Serial.println();
}

void handleMessage(Message msg){
  uint32_t len = 0;
  uint8_t* pbuf = msg.getInformation(&len);
  uint8_t to;
  uint8_t from;
  msg.getAdr(&from,&to);

  handlePacket(pbuf,len, from, to);
}

void handlePacket(uint8_t* pbuf, uint32_t len, uint8_t from, uint8_t to){
  uint8_t type = pbuf[0]>>4;
  Serial.printf("Handling a P%d",type);
  switch(type){
    case 2: handleP2(pbuf,len,from); break;
  }
}

// HANDLE SENDING
bool sendAck(uint8_t to, uint8_t from){
  delay(100);
  uint8_t buf[] = {from,to,0 /*Checksum is irrelevant for ACKs*/,0x01};
  Serial.printf("Sending Ack from %d to %d",from,to);
  bool success = rf95.send(buf,4);
  rf95.waitPacketSent();
  return success;
}

// CHATGPT HELPED HERE
void hexStringToByteArray(const char* hexString, uint8_t* byteArray, uint8_t* bufLen) {
  uint8_t i = 0;
  const char* ptr = hexString;

  while (i < *bufLen && *ptr != '\0') {
    // Skip until we find a hex pair, or "0x" style marker
    while (*ptr && (*ptr == ' ' || *ptr == ',' || *ptr == ':')) ptr++;

    if (*ptr == '0' && (*(ptr + 1) == 'x' || *(ptr + 1) == 'X')) {
      ptr += 2;
    }

    if (!isxdigit(ptr[0]) || !isxdigit(ptr[1])) break;

    uint8_t hi = hexCharToNum(ptr[0]);
    uint8_t lo = hexCharToNum(ptr[1]);
    if (hi == 0xFF || lo == 0xFF) break;

    byteArray[i++] = (hi << 4) | lo;
    ptr += 2;
  }

  *bufLen = i;
}

// ChatGPT helped here
uint8_t hexCharToNum(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return 0xFF; // Invalid
}

void processCMD(const char* cmd){
  if(memcmp(cmd,"motor",5) == 0){
    cmd+=6;
    if(memcmp(cmd,"reset",5)==0){
      motor::reset();
    }
    if(memcmp(cmd,"move",4) == 0){
      cmd+=5;
      int step = atoi(cmd);
      motor::step(step);
    }
    if(memcmp(cmd,"goto",4) == 0){
      cmd+=5;
      int step = atoi(cmd);
      motor::goToStep(step);
    }
    if(memcmp(cmd,"read",4) == 0){
      cmd+=5;
      Serial.printf("Encoder Location: %d\n", pid::encoderPos);
      Serial.printf("Step Location: %d\n",motor::getSteps());
      Serial.printf("Current Goal: %f\n",pid::setpoint);
    }
    if(memcmp(cmd,"power",5) == 0){
      Serial.printf("Power: %.2f\n",pid::output);
    }
    if(memcmp(cmd,"CTRL", 4) == 0){
      cmd+=5;
      if(memcmp(cmd,"power",5) == 0){
        cmd+=6;
        int power = atoi(cmd);
        pid::driveMotor(power);
        Serial.printf("Setting power to %d...\n",power);
        delay(10000);
      }
      if(memcmp(cmd,"PID",3) == 0){
        pid::runMotor = !pid::runMotor;
        if(pid::runMotor){
          Serial.println("PID ACTIVE");
        }else{
          Serial.println("PID INACTIVE");
        }
      }
    }
  }
  if(memcmp(cmd,"debug",5) == 0){
    cmd+=6;
    if(memcmp(cmd,"limit",5) == 0){
      if(motor::isPressed()){
        Serial.println("IS NOT PRESSED");
      }else{
        Serial.println("IS PRESSED");
      }
    }
    if(memcmp(cmd,"simPacket",9) == 0){
      cmd+=10;
      Serial.print("Testing ");
      uint8_t len = strlen(cmd) / 5;
      uint8_t CMDBUF[len];
      hexStringToByteArray(cmd,CMDBUF,&len);
      for(int i = 0; i < len; i++){
        Serial.printf("%d ", CMDBUF[i]);
        delay(5);
      }
      Serial.println();
      Serial.println(recv(CMDBUF,&len));
    }
    if(memcmp(cmd,"echo",4) == 0){
      cmd+=5;
      Serial.print(cmd);
    }
    if(memcmp(cmd,"data",4) == 0){
      Serial.printf("My Adr: %d, Server Adr: %d, InSystem: %d\n", myAdr, serverAdr, inSystem);
    }
    if(memcmp(cmd,"readR",5) == 0){
      cmd+=6;
      Serial.printf("R2: %.2f\n",reading::readResistance());
    }
    if(memcmp(cmd,"readV",5) == 0){
      cmd+=6;
      uint16_t data[0xff];
      for(int i = 0; i < 0xff; i++){
        data[i] = reading::readV();
      }
      float meanV = 0;
      for(int i = 0; i < 0xff; i++){meanV += data[i];} meanV /= 255.0;
      float stdDev = 0;
      for(int i = 0; i < 0xff; i++){stdDev += (data[i] - meanV) * (data[i] - meanV);}
      stdDev /= 0xff;
      stdDev = sqrt(stdDev);

      float VoltageF = (meanV * 3.3) / (1<<10);
      float R2 = R1 * (1 / ((MAX_VOLTAGE / VoltageF) - 1));
      Serial.printf("Mean: %.2f\nStdDev: %.2f\nVoltage: %.2f\nR2: %.2f\n",meanV,stdDev,VoltageF,R2);
      Serial.printf("%.2f, %.2f, %.2f, %.2f\n\n",meanV,stdDev,VoltageF,R2);
    }
  }
}