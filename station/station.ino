#include "comm.hpp"
#include "shared.hpp"

typedef struct segment{
  char msg [10];
} segment;

void callback(byte *data){
  segment s;
  memcpy(&s,data,sizeof(s));
  Serial.println(s.msg);
}

void setup() {
  // put your setup code here, to run once:
  registerCallbackRcv(callback);
  if (initESPNOWComms()){
    Serial.println("established");
    Serial.print("node id: ");
    Serial.println(espNowDataSend.nodeId);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
