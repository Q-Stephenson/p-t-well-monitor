#include "Client.h"

std::vector<Client> clients;

namespace clients_utils{
  uint8_t getAddress(String identifier){
    Client* cli = getByIdentifier(identifier);
    if(cli == NULL){
      return 0xff;
    }else{
      return (*cli).getAdr();
    }
  }
  Client* getByAddress(uint8_t adr){
    for(Client& cli : clients) if(cli.getAdr() == adr){return &cli;}
    return NULL;
  }
  Client* getByName(String name){
    for(Client& cli : clients) if(cli.getName().equals(name)){return &cli;}
    return NULL;
  }
  Client* getByIdentifier(String identifier){
    long int clientAdr = identifier.toInt();
    if(clientAdr == 0){
      return getByName(identifier);
    }else{
      return getByAddress((uint8_t) clientAdr);
    }
  }
}
