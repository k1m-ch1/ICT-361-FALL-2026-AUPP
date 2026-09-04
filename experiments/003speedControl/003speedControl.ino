#include "config.h"
#include "rc.h"

void setup(){
  Serial.begin(115200);
  pinMode(remoteControlPins.up, INPUT);
  pinMode(remoteControlPins.down, INPUT);
  pinMode(remoteControlPins.left, INPUT);
  pinMode(remoteControlPins.right, INPUT);

  xTaskCreate(
      debounceThenUpdate,
      "Debounce Task",
      2048,
      nullptr,
      1,
      &debounceThenUpdateTaskHandle // pass in the address of the task handle variable for it to store it inside
  );

  attachInterruptArg(
      remoteControlPins.left,
      buttonISR,
      (void *) remoteControlPins.left,
      BUTTON_INTERRUPT_MODE
  );

  attachInterruptArg(
      remoteControlPins.right,
      buttonISR,
      (void *) remoteControlPins.right,
      BUTTON_INTERRUPT_MODE
  );

  attachInterruptArg(
      remoteControlPins.up,
      buttonISR,
      (void *) remoteControlPins.up, // write the pin number as a uint32_t and pass it to the arg to propagate to the task
      BUTTON_INTERRUPT_MODE // is probably falling edge
  );

  attachInterruptArg(
      remoteControlPins.down,
      buttonISR,
      (void *) remoteControlPins.down, 
      BUTTON_INTERRUPT_MODE
  );
}

void loop(){
}
