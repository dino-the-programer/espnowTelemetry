#include "comm.hpp"
#include "shared.hpp"
#include <Arduino.h>

/* -------------------- Globals -------------------- */

volatile bool serialRxReady = false;
bool serialFlag = false;

serialMessage serialDataRcv;
serialMessage serialDataSend;

rx_frame_t    rxFrame;

/* ------------------------------------------------- */
/*               UART RECEIVE CALLBACK               */
/* ------------------------------------------------- */
/*
 * Runs in UART context.
 * MUST be fast, non-blocking, no ESP-NOW, no delays.
 */
void UART0_RX_CB() {
  static size_t idx = 0;

  while (Serial.available()) {
    uint8_t b = Serial.read();
    ((uint8_t*)&serialDataRcv)[idx++] = b;

    if (idx >= sizeof(serialDataRcv)) {
      idx = 0;
      serialRxReady = true;   // notify main loop
    }
  }
}

/* ------------------------------------------------- */
/*          SERIAL MESSAGE EVALUATION (SAFE)         */
/* ------------------------------------------------- */
/*
 * Call this from loop() or a FreeRTOS task
 */
void evalSerialComms() {
  if (!serialRxReady) return;
  serialRxReady = false;

  switch (serialDataRcv.command) {

    case serialCommand::SAVE:
      savePeer();
      break;

    case serialCommand::SEND:
      espNowDataSend.command = espNowCommand::DATA;
      memcpy(
        espNowDataSend.data,
        serialDataRcv.data,
        sizeof(espNowDataSend.data)
      );

      esp_now_send(
        serialDataRcv.header.broadcastAddress,
        (uint8_t*)&espNowDataSend,
        sizeof(espNowDataSend)
      );
      espNowDataSend.messageId++;
      break;

    case serialCommand::INIT:
      espNowDataSend.command = espNowCommand::INIT_CMD;
      memcpy(
        espNowDataSend.data,
        serialDataRcv.data,
        sizeof(espNowDataSend.data)
      );

      esp_now_send(
        serialDataRcv.header.broadcastAddress,
        (uint8_t*)&espNowDataSend,
        sizeof(espNowDataSend)
      );
      espNowDataSend.messageId++;
      break;

    default:
      // ignore unknown commands
      break;
  }
}


/* ------------------------------------------------- */
/*                   SAVE PEER                       */
/* ------------------------------------------------- */

void savePeer() {
  memcpy(peerInfo.peer_addr, serialDataRcv.header.broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    serialDataSend.command = serialCommand::FAILED;
    sendSerialComms();
  }
}

/* ------------------------------------------------- */
/*                SERIAL TRANSMIT                    */
/* ------------------------------------------------- */

void sendFramedSerial(uint8_t type, const uint8_t* payload, uint16_t len) {
  uint32_t crc = esp_crc32_le(0, &type, 1);
  crc = esp_crc32_le(crc, (uint8_t*)&len, 2);
  crc = esp_crc32_le(crc, payload, len);

  Serial.write(PREAMBLE_H);
  Serial.write(PREAMBLE_L);
  Serial.write(type);
  Serial.write((uint8_t*)&len, 2);
  Serial.write(payload, len);
  Serial.write((uint8_t*)&crc, 4);
  Serial.write(POSTAMBLE_H);
  Serial.write(POSTAMBLE_L);
}


void sendSerialComms() {
  uint8_t *payload = (uint8_t *)&serialDataSend;
  uint16_t len = sizeof(serialMessage);

  sendFramedSerial(FRAME_SERIAL, payload, len);
}

uint32_t esp_crc32_le(uint32_t crc, const uint8_t *buf, size_t len)
{
    crc = ~crc;   // invert initial CRC

    while (len--) {
        crc ^= *buf++;
        for (int i = 0; i < 8; i++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return ~crc;  // final invert
}

uint32_t calc_frame_crc(uint8_t type, uint16_t length, uint8_t *payload)
{
    uint32_t crc = 0;

    crc = esp_crc32_le(crc, &type, 1);
    crc = esp_crc32_le(crc, (uint8_t *)&length, 2);
    crc = esp_crc32_le(crc, payload, length);

    return crc;
}

bool uart_receive_frame(rx_frame_t *frame)
{
    static enum {
        WAIT_PREAMBLE_H,
        WAIT_PREAMBLE_L,
        READ_TYPE,
        READ_LEN_0,
        READ_LEN_1,
        READ_PAYLOAD,
        READ_CRC,
        READ_POST_0,
        READ_POST_1
    } state = WAIT_PREAMBLE_H;

    static uint16_t index = 0;
    static uint32_t rx_crc = 0;
    static uint8_t crc_bytes[4];

    while (Serial.available()) {
        uint8_t b = Serial.read();
        if (serialDataRcv.header.length > sizeof(serialDataRcv.data)) {
          return false;
        }

        switch (state) {

        case WAIT_PREAMBLE_H:
            if (b == PREAMBLE_H) state = WAIT_PREAMBLE_L;
            break;

        case WAIT_PREAMBLE_L:
            state = (b == PREAMBLE_L) ? READ_TYPE : WAIT_PREAMBLE_H;
            break;

        case READ_TYPE:
            frame->type = b;
            state = READ_LEN_0;
            break;

        case READ_LEN_0:
            frame->length = b;
            state = READ_LEN_1;
            break;

        case READ_LEN_1:
            frame->length |= (b << 8);
            if (frame->length > MAX_PAYLOAD) {
                state = WAIT_PREAMBLE_H;
            } else {
                index = 0;
                state = READ_PAYLOAD;
            }
            break;

        case READ_PAYLOAD:
            frame->payload[index++] = b;
            if (index == frame->length) {
                index = 0;
                state = READ_CRC;
            }
            break;

        case READ_CRC:
            crc_bytes[index++] = b;
            if (index == 4) {
                memcpy(&rx_crc, crc_bytes, 4);
                index = 0;
                state = READ_POST_0;
            }
            break;

        case READ_POST_0:
            state = (b == POSTAMBLE_H) ? READ_POST_1 : WAIT_PREAMBLE_H;
            break;

        case READ_POST_1:
            state = WAIT_PREAMBLE_H;
            if (b != POSTAMBLE_L) break;

            uint32_t calc_crc = calc_frame_crc(
                frame->type,
                frame->length,
                frame->payload
            );

            if (calc_crc == rx_crc) {
                return true;   // ✅ VALID FRAME
            }
            break;
        }
    }
    return false;
}

void pollSerial()
{
    if (!uart_receive_frame(&rxFrame)) return;

    if (rxFrame.type != FRAME_SERIAL) return;

    // Validate payload size
    if (rxFrame.length != sizeof(serialMessage)) return;

    // Copy validated payload into struct
    memcpy(&serialDataRcv, rxFrame.payload, sizeof(serialMessage));

    serialRxReady = true;
}




/* ------------------------------------------------- */
/*               SERIAL INITIALIZATION               */
/* ------------------------------------------------- */

bool initSerialComms() {
  // Serial.begin(115200);
  // Serial.onReceive(UART0_RX_CB, true);
  // while(!Serial.available()){}
  // Serial.readString();
  serialDataSend.command = serialCommand::STARTED;
  sendSerialComms();

  return true;
}
