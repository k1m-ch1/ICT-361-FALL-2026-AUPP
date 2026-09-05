#pragma once

#include "logging.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <Arduino.h>

QueueHandle_t logQueueHandle;

void loggingInit() {
  // NOTE: remember to init this as boilerplate. More responsibility, definitely
  // isn't pure and definitely is causing side effects, but it seems like this
  // is common in embedded programming...
  logQueueHandle = xQueueCreate(LOG_QUEUE_SIZE, sizeof(LogMessage));
}

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
    xQueueReceive(logQueueHandle, &logMessage, portMAX_DELAY);
    // so that we don't need to do \r\n every time.
    Serial.printf("[%lu] [%s] %s\r\n", logMessage.timestamp,
                  getLogSourceName(logMessage.logSource), logMessage.text);
  }
}
