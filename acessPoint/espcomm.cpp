#include "comm.hpp"
#include "shared.hpp"

esp_now_peer_info_t peerInfo;
espNowMessage espNowDataRcv;
espNowMessage espNowDataSend;
bool espnowFlag = false;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  espnowFlag = false;
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  while(serialFlag){};
  serialFlag = true;
  memcpy(serialDataSend.header.broadcastAddress,mac,6);
  memcpy(&espNowDataRcv, incomingData, sizeof(espNowDataRcv));
  evalEspComms();
}

bool initESPNOWComms(){
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    return false;
  }
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  
  espNowDataSend.nodeId = 0;
  espNowDataSend.messageId = 0;
  return true;
}