#ifndef STATION_TYPES
#define STATION_TYPES

enum espNowCommand : uint32_t {
  SYNC,
  ACK,
  DATA,
  INIT_CMD,
};

typedef struct {
  uint16_t nodeId;
  uint8_t messageId;
  uint32_t command;
  uint8_t data[200];
} espNowMessage;

typedef struct configNode{
  int nodeId;
} configNode;

typedef void (*callBackRcv)(byte *);

#endif