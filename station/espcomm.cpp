#include "comm.hpp"
#include "shared.hpp"
// #include "config.hpp"
#include <esp_wifi.h>

esp_now_peer_info_t peerInfo;
bool connectionTry = true;
callBackRcv cbRCV;
espNowMessage espNowDataRcv;
espNowMessage espNowDataSend;
uint8_t broadcastAddress[] = {0x4C, 0xC3, 0x82, 0xBF, 0x6C, 0xD4};

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&espNowDataRcv, incomingData, sizeof(espNowDataRcv));
  if (espNowDataRcv.command == espNowCommand::DATA){
    cbRCV(espNowDataRcv.data);
  }
  else if(espNowDataRcv.command == espNowCommand::ACK){
    connectionTry = false;
  }
  else if(espNowDataRcv.command == espNowCommand::INIT_CMD){
    connectionTry = false;
    configNode cn;
    memcpy(&cn,espNowDataRcv.data,sizeof(cn));
    espNowDataSend.nodeId = cn.nodeId;
  }
}

void sendData(byte *data, int size){
  espNowDataSend.command = espNowCommand::DATA;
  memcpy(espNowDataSend.data,data,size);
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &espNowDataSend, sizeof(espNowDataSend));
  espNowDataSend.messageId++;
}

void registerCallbackRcv(callBackRcv cbrcv){
  cbRCV = cbrcv;
}

bool initESPNOWComms(){
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    return false;
  }
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    return false;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  espNowDataSend.nodeId = -1;
  espNowDataSend.command = espNowCommand::SYNC;
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    memcpy(espNowDataSend.data,baseMac,6);
  }
  while (connectionTry){
    espNowDataSend.messageId = 0;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &espNowDataSend, sizeof(espNowDataSend));
    espNowDataSend.messageId++;
    delay(2000);
  }
  return true;
}