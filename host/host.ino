// Client side of the system
#include <stdio.h>
#include <hardware/gpio.h>
#include <string>
#include <cstring> // Include for memcpy
#include "RH_RF95.h"
#include "Message.h"
#include "Client.h"
#include "pico/unique_id.h"

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

uint8_t serverAdr = 0; // who is also me

uint8_t clientAdrGen = 1;
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

  if(toAdr == serverAdr && toAdr != BROADCAST_ADDRESS){
    if(checksum != runningsum){
      // Leaves and expects a resend
      Serial.println("Failed Checksum");
      return false;
    }
    if(toAdr != BROADCAST_ADDRESS){
      sendAck(fromAdr,toAdr);
      Serial.println("Sent ACK");
    }
  }

  if(toAdr == BROADCAST_ADDRESS || toAdr == serverAdr){
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
      // Serial.println("Sending");
      // for(int j = 0; j < len + ADDRESS_HEADER_LENGTH + MESSAGE_HEADER_LENGTH; j++) Serial.printf("%d ", sbuf[j]);
      // Serial.println();

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
    sbuf[4]++;
  }
  return true;
}

void setup()
{
  delay(1000);
  randomSeed(analogRead(A0));
  Serial.begin(115200);
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

  Serial.println("Completed Setup");
}

void loop(){
  handleMSGs();

  if(Serial.available()){
    String receivedData = Serial.readString();
    processCMD(receivedData.c_str());
  }

  // Check if a message is available
  if (rf95.available()) {
    Serial.println("Received a packet");
    // Receive the message
    uint8_t len = sizeof(buf);
    rf95.recv(buf,&len);
    recv(buf,&len);
  }

  // Pings each client and disconnects it if it does not respond (every 10 minutes)
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
    case 0:{
      // Initiates a CCS
      uint64_t identity;
      memcpy((uint8_t*) &identity, pbuf+1, 8);

      uint8_t clientAdr = 0;
      while(clientAdr == 0 || clientAdr == BROADCAST_ADDRESS /*TODO: Other tests to make sure that the address is not occupied*/) clientAdr = clientAdrGen++;

      uint8_t sbuf[11];
      sbuf[0] = 0x10;
      memcpy(sbuf+1,pbuf+1,8);
      Serial.println();
      sbuf[9] = clientAdr;
      sbuf[10] = serverAdr;

      delay(100);

      rf95.send(sbuf,11);

      Serial.printf("CLIENT WITH %d\n",clientAdr);

      Client* cli = clients_utils::getByAddress(clientAdr);
      if(cli == NULL)
        clients.push_back(Client(clientAdr));
    }break;
    case 3:{
      Serial.printf("-R%d-C%d-%d\n",pbuf[0] & 0xF,pbuf[6]>>4,pbuf[6] & 0xF);
      uint8_t RType = pbuf[0] & 0xF;
      Client* cli = clients_utils::getByAddress(from);
      if(cli == NULL){
        Serial.printf("FAILED TO IDENTIFY CLIENT @ %d\n",from);
      }
      switch(RType){
        case 2:
        (*((*cli).getReadings())).uploadR2(pbuf+8, 0xf);
        break;
      }
    }break;
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

uint8_t chopAddressFromCMD(const char** cmd){
  uint8_t TOADR;
  uint8_t chopto = 0;
  while((*cmd)[chopto] != ' ') chopto++;
  char identifier[chopto + 1];
  memcpy(identifier,(*cmd),chopto);
  identifier[chopto] = '\0';
  TOADR = clients_utils::getAddress(String(identifier));
  (*cmd)+=chopto + 1;
  return TOADR;
}

void processCMD(const char* cmd){
  if(memcmp(cmd,"motor",5) == 0){
    cmd+=6;
    if(memcmp(cmd,"move",4) == 0){
      cmd+=5;
      while(isspace(cmd[0]) && cmd[0] != '\0')cmd++;
      if(cmd[0] == '\0') return;
      
      uint8_t TOADR = chopAddressFromCMD(&cmd);
      {
        if(TOADR == 0xff){
          Serial.println("Failed to PARSE TOADDRESS");
          return;
        }
      }

      char CMD[100] = "send to ";
      itoa(TOADR,CMD+8,10);
      strcat(CMD," P2-C8-M1 (");
      strcat(CMD,cmd);
      strcat(CMD,")");
      processCMD(CMD);
      return;
    }    
    if(memcmp(cmd,"reset",5) == 0){
      cmd+=6;
      while(isspace(cmd[0]) && cmd[0] != '\0')cmd++;
      if(cmd[0] == '\0') return;
      
      uint8_t TOADR = chopAddressFromCMD(&cmd);
      {
        if(TOADR == 0xff){
          Serial.println("Failed to PARSE TOADDRESS");
          return;
        }
      }

      char CMD[100] = "send to ";
      itoa(TOADR,CMD+8,10);
      strcat(CMD," P2-C0");
      processCMD(CMD);
    }
  }
  if(memcmp(cmd,"name",4) == 0){
    cmd+=5;
    long int clientAdr = 0;
    char* name;
    clientAdr = strtol(cmd,&name,10);
    Client* cli = clients_utils::getByAddress((uint8_t) clientAdr);
    if(cli == NULL){
      Serial.printf("FAILED, adr: %d is empty", clientAdr);
    }else{
      trim(name);
      (*cli).setName(String(name));
    }
  }
  if(memcmp(cmd,"send",4) == 0){
    cmd+=5;
    if(memcmp(cmd,"payload",7) == 0){
      cmd+=8;
      uint8_t len = strlen(cmd) / 5;
      uint8_t CMDBUF[len];
      hexStringToByteArray(cmd,CMDBUF,&len);

      uint8_t to = CMDBUF[0];
      uint8_t* bufref = CMDBUF + 1; len--;
      send(bufref,len,to,serverAdr);
      return;
    }
    if(memcmp(cmd,"to",1) == 0){
      cmd+=3;
      uint8_t TOADR = chopAddressFromCMD(&cmd);

      if(TOADR == 0xff){
        Serial.println("Failed to PARSE TOADDRESS");
        return;
      }

      Serial.println();

      for(int i = 0; i < 10 && cmd[i] != '\0'; i++) Serial.print(cmd[i]);

      Serial.println();

      uint8_t Ptype = hexCharToNum(cmd[1]);
      if(Ptype == 0xff){
        Serial.println("Failed to Parse Ptype");
        return;
      }

      switch(Ptype){
        case 2:{
          uint8_t Ctype = hexCharToNum(cmd[4]);
          uint8_t PDATA[11];
          PDATA[0] = (Ptype<<4) | Ctype;
          uint32_t time = millis();
          memcpy(PDATA+1,(uint8_t*) &time, 4);
          PDATA[5] = 0 /*TO BE CHANGED*/;
          switch(Ctype){
            case 0: case 1: case 2: case 3: case 4: case 6: case 7:  {
              send(PDATA,5,TOADR,serverAdr);
            }
            break;
            case 5: {
              uint8_t subcmd = hexCharToNum(cmd[6]);
              if(subcmd == 0xff){
                uint8_t pt = 6;
                subcmd = 0;
                while(cmd[pt] != '\0'){
                  switch(cmd[pt]){
                    case 'W': case 'w': subcmd |= 8; break;
                    case 'P': case 'p': subcmd |= 4; break;
                    case 'S': case 's': subcmd |= 2; break;
                    case 'T': case 't': subcmd |= 1; break;
                  }
                  pt++;
                }
              }
              PDATA[6] = subcmd<<4;
              send(PDATA,6,TOADR,serverAdr);
            }break;
            case 8: {
              uint8_t Mtype = hexCharToNum(cmd[7]);
              uint32_t arg1 = 0;
              PDATA[6] = (Mtype & 0xf)<<4;
              uint32_t pnt = 7;
              while(cmd[pnt] != '(' && cmd[pnt] != '\0') pnt++;
              if(cmd[pnt] == '('){
                char* end;
                arg1 = strtol(cmd+pnt+1,&end,10);
              }
              memcpy(PDATA+7,(uint8_t*) &arg1,4);
              Serial.printf("%d to %d P%d-C%d-M%d (%d)",serverAdr,TOADR,Ptype,Ctype,Mtype,arg1);
              send(PDATA,11,TOADR,serverAdr);
            }
            break;
          }
        } break;
      }
    }
  }
  if(memcmp(cmd,"debug",5) == 0){
    cmd+=6;
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
    if(memcmp(cmd,"list",4) == 0){
      for(Client cli : clients){
        Serial.println(cli.getString());
      }
    }
  }
  if(memcmp(cmd,"loop",4) == 0){
    cmd+=5;
    long int count = 0;
    char* lastCMD;
    count = strtol(cmd,&lastCMD,10);
    // Serial.printf("\nLoop Count: %d\n",count);
    // Serial.println(lastCMD);
    while(lastCMD[0] == ' ') lastCMD++;
    for(int i = 0; i < count; i++) processCMD(lastCMD);
  }
}

// Trims leading and trailing whitespace from a C-string
void trim(char* str) {
    if (str == nullptr) return;

    // Step 1: Trim leading whitespace
    char* start = str;
    while (std::isspace(static_cast<unsigned char>(*start))) {
        ++start;
    }

    // Step 2: Trim trailing whitespace
    char* end = start + std::strlen(start) - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(*end))) {
        *end = '\0';
        --end;
    }

    // Step 3: Shift the trimmed string to the beginning if needed
    if (start != str) {
        std::memmove(str, start, std::strlen(start) + 1);  // Include null terminator
    }
}
