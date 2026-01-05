#include "comm.hpp"
#include "shared.hpp"

esp_now_peer_info_t peerInfo;
espNowMessage espNowDataRcv;
espNowMessage espNowDataSend;
bool espnowFlag = false;
volatile bool espNowRxReady = false;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  espnowFlag = false;
}

void OnDataRecv(const uint8_t *mac,const uint8_t *incomingData,int len)
{
  if (len != sizeof(espNowMessage)) return;
  if (espNowRxReady) return;   // drop if busy

  memcpy(&espNowDataRcv, incomingData, sizeof(espNowMessage));
  // memcpy(serialDataSend.header.broadcastAddress, mac, 6);

  espNowRxReady = true;
}

void evalEspComms()
{
  if (!espNowRxReady) return;
  espNowRxReady = false;

  switch (espNowDataRcv.command) {

    case espNowCommand::SYNC:
      serialDataSend.command = serialCommand::SAVE;
      memcpy(serialDataSend.header.broadcastAddress, espNowDataRcv.data, 6);

      // Optional: include sender MAC already copied in callback
      // serialDataSend.header.broadcastAddress is already set

      sendSerialComms();   // sends framed + CRC serial packet
      break;

    default:
      // ignore other commands
      break;
  }
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