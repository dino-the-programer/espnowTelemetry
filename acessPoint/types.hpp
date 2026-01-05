#ifndef STATION_TYPES
#define STATION_TYPES

#define MAX_PAYLOAD 256

enum serialCommand : uint32_t {
  SEND,
  SENT,
  FAILED,
  SAVE,
  INIT,
  STARTED,
};

enum espNowCommand : uint32_t {
  SYNC,
  ACK,
  DATA,
  INIT_CMD,
};

typedef struct {
  uint8_t broadcastAddress[6];
  uint32_t length;
} dataHeader;


typedef struct {
  uint32_t command;
  dataHeader header;
  uint8_t data[200];
} serialMessage;

typedef struct {
  uint16_t nodeId;
  uint8_t messageId;
  uint32_t command;
  uint8_t data[200];
} espNowMessage;

typedef struct {
    uint8_t  type;
    uint16_t length;
    uint8_t  payload[MAX_PAYLOAD];
} rx_frame_t;

typedef struct configNode{
  int nodeId;
} configNode;


#endif