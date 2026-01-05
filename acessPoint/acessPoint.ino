#include "comm.hpp"

TaskHandle_t serialCommsTaskHandle = NULL;
TaskHandle_t espNowTaskHandle = NULL;

void serialCommsTask(void *parameter){
  while(1){
    pollSerial();
    evalSerialComms();
    vTaskDelay(1);
  }
}

void espNowTask(void *parameter){
  while(1){
    evalEspComms();
    vTaskDelay(1);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  xTaskCreatePinnedToCore(
    serialCommsTask,         // Task function
    "UART COMMS",       // Task name
    10000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &serialCommsTaskHandle,  // Task handle
    1                  // Core 1
  );
  initSerialComms();
  xTaskCreatePinnedToCore(
    espNowTask,         // Task function
    "ESPNOW COMMS",       // Task name
    10000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &espNowTaskHandle,  // Task handle
    0                  // Core 1
  );
  initESPNOWComms();

}

void loop() {
  // put your main code here, to run repeatedly:
}
