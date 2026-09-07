#include "config.h"
#include "rc.h"
#include "logging.h"

void setup(){
  rcInit();
  loggingInit();

  xTaskCreate(
      pollButtonTask,
      "Polling button Task",
      2048,
      nullptr,
      1,
      nullptr
  );// no need to store the task handle
}

void loop(){
}
