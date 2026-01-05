#ifndef STATION_COMM
#define STATION_COMM

#include <esp_now.h>
#include <WiFi.h>

#include "types.hpp"

#define PREAMBLE_H 0xAA
#define PREAMBLE_L 0x55
#define POSTAMBLE_H 0x55
#define POSTAMBLE_L 0xAA

#define FRAME_SERIAL  0x01
#define FRAME_ESPNOW  0x02
#define FRAME_CONFIG  0x03


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

bool initESPNOWComms();

void UART0_RX_CB();
void savePeer();
void sendFramedSerial(uint8_t type, const uint8_t* payload, uint16_t len);
uint32_t esp_crc32_le(uint32_t crc, const uint8_t *buf, size_t len);
uint32_t calc_frame_crc(uint8_t type, uint16_t length, uint8_t *payload);
bool uart_receive_frame(rx_frame_t *frame);
void pollSerial();
void sendSerialComms();
void evalSerialComms();
void evalEspComms();
bool initSerialComms();

#endif