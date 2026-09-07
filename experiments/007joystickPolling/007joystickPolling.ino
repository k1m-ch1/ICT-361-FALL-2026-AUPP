#include "config.h"
#include "rc.h"
#include "logging.h"

#define JOYSTICK_LOG_PERIOD 250

LogMessage joystickLogMessage;

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

  xTaskCreate(
      pollJoystickTask,
      "Polling joystick Task",
      2048,
      nullptr,
      1,
      nullptr
  );// no need to store the task handle
}

void loop(){
  joystickLogMessage.timestamp = millis();
  joystickLogMessage.logSource = RC;
  xSemaphoreTake(joystickStateMutex, portMAX_DELAY); // wait indefinitely
  sprintf(joystickLogMessage.text, "Joystick x: %lu, y: %lu", joystickState.x, joystickState.y);
  xSemaphoreGive(joystickStateMutex);
  xQueueSend(logQueueHandle, &joystickLogMessage, 0);
  delay(JOYSTICK_LOG_PERIOD);
}
