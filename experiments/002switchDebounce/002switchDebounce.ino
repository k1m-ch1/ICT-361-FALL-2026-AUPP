#include "config.h"
#include "rc.h"

void setup(){
  Serial.begin(115200);

  xTaskCreate(
      debounceThenUpdate,
      "Up Debounce",
      2048,
      nullptr,
      1,
      &debounceThenUpdateTaskHandle // pass in the address of the task handle variable for it to store it inside
  );

  attachInterruptArg(
      remoteControlPins.up,
      buttonISR,
      (void *) remoteControlPins.up, // write the pin number as a uint32_t and pass it to the arg to propagate to the task
      FALLING // since the button is normally HIGH
  );
}

void loop(){
}
