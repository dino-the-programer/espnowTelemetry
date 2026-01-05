#ifndef STATION_ESPCOMM
#define STATION_ESPCOMM

#include <esp_now.h>
#include <WiFi.h>

#include "types.hpp"

void registerCallbackRcv(callBackRcv cbrcv);

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

bool initESPNOWComms();

#endif