#ifndef STATION_TYPES
#define STATION_TYPES

enum class serialCommand{
  SEND,
  SENT,
  FAILED,
  SAVE,
  INIT,
};

enum class espNowCommand{
  SYNC,
  ACK,
  DATA,
  INIT,
};

typedef struct dataHeader{
  uint8_t broadcastAddress[6];
  int lenght;
} header;

typedef struct serialMessage{
  serialCommand command;
  dataHeader header;
  byte data[200];
} serialMessage;

typedef struct epsNowMessage{
  int nodeId;
  uint8_t messageId;
  espNowCommand command;
  byte data[200];
} espNowMessage;

typedef struct configNode{
  int nodeId;
} configNode;


#endif