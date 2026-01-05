#include "comm.hpp"
#include "shared.hpp"

bool serialFlag = false;

serialMessage serialDataRcv;
serialMessage serialDataSend;

void UART0_RX_CB() {
  byte buff[sizeof(serialDataRcv)];
  while (Serial.available()) {
    Serial.readBytes(buff,sizeof(serialDataRcv));
  }
  memcpy(&serialDataRcv,buff,sizeof(serialDataRcv));
  if (serialDataRcv.command == serialCommand::SAVE){
    savePeer();
  }
  else if (serialDataRcv.command == serialCommand::SEND){
    while(espnowFlag){};
    espnowFlag = true;
    espNowDataSend.command = espNowCommand::DATA;
    memcpy(espNowDataSend.data , serialDataRcv.data,sizeof(espNowDataSend.data));
    esp_err_t result = esp_now_send(serialDataRcv.header.broadcastAddress, (uint8_t *) &espNowDataSend, sizeof(espNowDataSend));
    espNowDataSend.messageId++;
  }
  else if(serialDataRcv.command == serialCommand::INIT){
    while(espnowFlag){};
    espnowFlag = true;
    espNowDataSend.command = espNowCommand::INIT;
    memcpy(espNowDataSend.data , serialDataRcv.data,sizeof(espNowDataSend.data));
    esp_err_t result = esp_now_send(serialDataRcv.header.broadcastAddress, (uint8_t *) &espNowDataSend, sizeof(espNowDataSend));
    espNowDataSend.messageId++;
  }
}

void evalEspComms(){
  if(espNowDataRcv.command==espNowCommand::SYNC){
    serialDataSend.command=serialCommand::SAVE;
    sendSerialComms();
  }
}

void savePeer(){
  memcpy(peerInfo.peer_addr, serialDataRcv.header.broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    while(serialFlag){};
    serialFlag=true;
    serialDataSend.command = serialCommand::FAILED;
    sendSerialComms();
  }
}

void sendSerialComms(){
  byte buff[sizeof(serialDataSend)];
  memcpy(&buff,&serialDataSend,sizeof(serialDataSend));
  Serial.write(buff,sizeof(serialDataSend));
  serialFlag=false;
}

bool initSerialComms(){
  Serial.begin(115200);
}