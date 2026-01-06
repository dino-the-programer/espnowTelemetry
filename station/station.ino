#include "comm.hpp"
#include "shared.hpp"
// #include "config.hpp"

// uint8_t broadcastaddr[] = {0x4C, 0xC3, 0x82, 0xBF, 0x6C, 0xD4};

typedef struct segment{
  char msg [10];
} segment;

typedef struct dataSend{
  int a;
} dataSend;

void callback(byte *data){
  segment s;
  memcpy(&s,data,sizeof(s));
  Serial.println(s.msg);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  registerCallbackRcv(callback);
  if (initESPNOWComms()){
    Serial.println("established");
    Serial.print("node id: ");
    Serial.println(espNowDataSend.nodeId);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  segment data;
  char *msg = "esp32\0";
  memcpy(data.msg,msg,6);
  sendData((byte *)&data,6);
  delay(10000);
}
