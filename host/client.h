#ifndef CLIENT_H
#define CLIENT_H

#include "reading.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <Arduino.h>

class Client {
  private:
    String name = "Unknown";
    uint8_t address = 0xff;
    Reading readings;

  public:
    Client() {}

    Client(uint8_t _adr)
      : address(_adr) {}

    Client(String _name, uint8_t _adr, Reading _read)
      : name(_name), address(_adr), readings(_read) {}


    String getName(){return name;}
    uint8_t getAdr(){return address;}
    Reading* getReadings(){return &readings;}
    void setName(String _name){name = _name;}

    String getString(){
      return "Name: " + getName() + ", address: " + String(address) + "\n" + readings.getString() + "\n";
    }
};

extern std::vector<Client> clients;

namespace clients_utils{
  uint8_t getAddress(String idenitifier);
  Client* getByAddress(uint8_t adr);
  Client* getByName(String name);
  Client* getByIdentifier(String identifier);
}

#endif
