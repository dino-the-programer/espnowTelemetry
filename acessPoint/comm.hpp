#ifndef STATION_COMM
#define STATION_COMM

#include <esp_now.h>
#include <WiFi.h>

#include "types.hpp"

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

bool initESPNOWComms();

void UART0_RX_CB();
void savePeer();
void sendSerialComms();
void evalEspComms();

#endif