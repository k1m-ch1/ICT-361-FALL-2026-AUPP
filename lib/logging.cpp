#pragma once

#include "logging.h"
#include "config.h"

QueueHandle_t logQueueHandle =
    xQueueCreate(LOG_QUEUE_LENGTH, sizeof(LogMessage));

const char *getLogSourceName(LogSource logSource) {
  switch (logSource) {
  case MOTOR:
    return "MOTOR";
  case MIXER:
    return "MIXER";
  case RC:
    return "RC";
  default:
    return "UNKNOWN";
  }
}

void loggingTask() {
  LogMessage logMessage;
  // this is blocking, so we create a seperate task
  while (true) {
    xQueueReceive(logQueue, &logMessage, portMAX_DELAY);
    // so that we don't need to do \r\n every time.
    Serial.printf("[%lu] [%s] %s\r\n", logMessage.timeStamp,
                  getLogSourceName(logMessage.logSource), logMessage.text);
  }
}
