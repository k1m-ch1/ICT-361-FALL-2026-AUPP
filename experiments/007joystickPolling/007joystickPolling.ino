#include "config.h"
#include "rc.h"
#include "logging.h"
#include "mixer.h"

#define JOYSTICK_LOG_PERIOD 250

LogMessage joystickLogMessage;

void setup(){
  loggingInit();
  mixerInit();
  rcInit();
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
