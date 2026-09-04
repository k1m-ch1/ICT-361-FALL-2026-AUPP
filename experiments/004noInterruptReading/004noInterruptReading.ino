#include "config.h"

void setup(){
  Serial.begin(115200);
  pinMode(remoteControlPins.up, INPUT);
  pinMode(remoteControlPins.down, INPUT);
  pinMode(remoteControlPins.left, INPUT);
  pinMode(remoteControlPins.right, INPUT);
}

void loop()
{
    Serial.printf(
        "U=%d D=%d L=%d R=%d\r\n",
        digitalRead(remoteControlPins.up),
        digitalRead(remoteControlPins.down),
        digitalRead(remoteControlPins.left),
        digitalRead(remoteControlPins.right)
    );

    delay(50);
}
