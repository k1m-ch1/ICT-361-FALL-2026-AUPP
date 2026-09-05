#pragma once

#include "config.h"
#include <stdint.h>

// 128 bytes for each log should be plenty
#define LOG_MESSAGE_SIZE 128
#define LOG_QUEUE_SIZE 8

typedef enum { MOTOR, MIXER, RC } LogSource;

typedef struct {
  uint32_t timeStamp;
  LogSource logSource;
  char text[LOG_MESSAGE_SIZE];
} LogMessage;

extern QueueHandle_t logQueue;

const char *getLogSourceName(LogSource logSource);
void loggingTask();
