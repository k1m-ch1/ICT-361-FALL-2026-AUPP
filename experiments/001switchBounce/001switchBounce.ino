#include <stdint.h>
#include "config.h"

uint32_t prevTimeStamp = 0;
uint32_t timeStamp = 0;
uint32_t count = 0;

void isr(){
  count++;
  timeStamp = millis();
}

void setup(){
  Serial.begin(921600);
  pinMode(remoteControlPins.up, INPUT);
  attachInterrupt(digitalPinToInterrupt(remoteControlPins.up), isr, FALLING);
}

void loop(){
  if (prevTimeStamp != timeStamp){
    Serial.printf("%d, %d\r\n", count, timeStamp);
    prevTimeStamp = timeStamp;
  }
}
