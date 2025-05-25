#ifndef MESSAGE_H
#define MESSAGE_H

#include "Addresser.h"

#define MESSAGE_HEADER_LENGTH 3
#define MAX_MESSAGE_COUNT 16
#define PAYLOAD_LEN (RH_RF95_MAX_MESSAGE_LEN - MESSAGE_HEADER_LENGTH - ADDRESS_HEADER_LENGTH)
#define UNIDENTIFIED_PACKET 0xff

class Message{
  private:
    uint8_t packetLen = 0;
    uint8_t* packets = NULL;
    uint8_t packetID = UNIDENTIFIED_PACKET;
    uint8_t packetCount = 0;
    bool initialized = 0;
    uint8_t to = 0xff;
    uint8_t from = 0xff;
  public:
    Message(uint8_t* pbuf, uint8_t len){init(pbuf,len);}

    Message(){}

    void init(uint8_t* pbuf, uint8_t len){
      if(packets != NULL) delete[] packets;

      packetID = pbuf[0];
      uint32_t offset = ((uint32_t) pbuf[1]) * PAYLOAD_LEN;
      packetLen = pbuf[2];
      
      size_t size = packetLen * PAYLOAD_LEN;

      packets = new (std::nothrow) uint8_t[size];
      if (packets == nullptr) {
        Serial.println("Memory allocation failed!");
        packetLen = 0;
        return;
      }

      memset(packets, 0, size);

      memcpy(packets+offset,pbuf+MESSAGE_HEADER_LENGTH,min(PAYLOAD_LEN,len - MESSAGE_HEADER_LENGTH));

      Serial.println();

      packetCount++;
      initialized = 1;
    }

    void addP4(uint8_t* pbuf, uint8_t len){
      if(!isInit()){init(pbuf,len); return;}

      if(pbuf[0] != packetID) return;

      memcpy(packets+PAYLOAD_LEN*pbuf[1],pbuf+3,PAYLOAD_LEN);
      packetCount++;
    }

    bool complete(){
      return packetCount >= packetLen;
    }

    uint8_t* getInformation(uint32_t* len){
      *len = packetLen * PAYLOAD_LEN;
      return packets;
    }

    void setAdr(uint8_t fromAdr = 0xff, uint8_t toAdr = 0xff){
      to = toAdr;
      from = fromAdr;
    }

    void getAdr(uint8_t* fromAdr = NULL, uint8_t* toAdr = NULL){
      if(fromAdr != NULL) *fromAdr = from;
      if(toAdr != NULL) *toAdr = to;
    }

    bool isInit(){
      return initialized;
    }

    uint8_t getPacketID() const {
        return packetID;
    }

    ~Message(){
      if(packets != NULL) delete[] packets;
    }

    // CHATGPT
    // Deep copy constructor
    Message(const Message& other) {
      packetLen = other.packetLen;
      packetID = other.packetID;
      packetCount = other.packetCount;
      initialized = other.initialized;
      to = other.to;
      from = other.from;

      size_t size = packetLen * PAYLOAD_LEN;
      packets = new (std::nothrow) uint8_t[size];
      if (packets && other.packets) {
        memcpy(packets, other.packets, size);
      }
    }

    // CHATGPT
    Message& operator=(const Message& other) = delete; // or implement deep copy too
};

Message* messages[MAX_MESSAGE_COUNT];

bool isIDInWaiting(uint8_t id){
  for(int i = 0; i < MAX_MESSAGE_COUNT; i++){
    if(messages[i] != NULL && messages[i]->getPacketID() == id) return true;
  }
  return false;
}

Message* getByID(uint8_t id){
  for(int i = 0; i < MAX_MESSAGE_COUNT; i++){
    if(messages[i] != NULL && messages[i]->getPacketID() == id) return messages[i];
  }
  return NULL;
}

void handleMessage(Message msg);

void handleMSGs(){
  for(int i = 0; i < MAX_MESSAGE_COUNT; i++){
    if(messages[i] != NULL && messages[i]->complete()){
      Message msg = Message(*messages[i]);
      handleMessage(msg);
      delete messages[i];
      messages[i]=NULL;
    }
  }
}

#endif