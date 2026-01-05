#include "comm.hpp"
#include "shared.hpp"
#include "config.hpp"

esp_now_peer_info_t peerInfo;
bool connectionTry = true;
callBackRcv cbRCV;
espNowMessage espNowDataRcv;
espNowMessage espNowDataSend;

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
  else if(espNowDataRcv.command == espNowCommand::INIT){
    connectionTry = false;
    configNode cn;
    memcpy(&cn,espNowDataRcv.data,sizeof(cn));
    espNowDataSend.nodeId = cn.nodeId;
  }
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
  while (connectionTry){
    espNowDataSend.messageId = 0;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &espNowDataSend, sizeof(espNowDataSend));
    espNowDataSend.messageId++;
    delay(2000);
  }
  return true;
}