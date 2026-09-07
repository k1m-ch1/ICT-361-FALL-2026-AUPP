#pragma once

#include "mixer.h"
#include "config.h"
#include "logging.h"
#include "math.h"
#include "utils.h"
#include <cstdlib>

Speed speed;

SpeedLimit speedLimit;

SemaphoreHandle_t normalizedMotorCommandsMutex;
float normalizedMotorCommands[MOTORS_AMOUNT];

SemaphoreHandle_t speedMutex;
SemaphoreHandle_t speedLimitMutex;

TaskHandle_t mixerTaskHandle;

void mixerInit() {
  speed = {.linear = 0, .angular = 0};
  speedLimit = {.linear = 0.5f, .angular = 0.5f};
  for (uint8_t i = 0; i < MOTORS_AMOUNT; i++) {
    normalizedMotorCommands[i] = 0;
  }
  speedMutex = xSemaphoreCreateMutex();
  speedLimitMutex = xSemaphoreCreateMutex();
  normalizedMotorCommandsMutex = xSemaphoreCreateMutex();
  xTaskCreate(mixerTask, "Mixer Task", 4096, nullptr, 1, &mixerTaskHandle);
}

void mixerTask(void *args) {
  // mixer task should be notification based

  // LogMessage mixerLogMessage;
  float outputCommand[OUTPUT_COMMAND_DIM];
  float absOutputCommandMax;

  // zero everything out first, this is our intermediate, if we have a
  // differential drive robot, OUTPUT_COMMAND_DIM = 2, zeroth index will be
  // left, first index will right
  for (uint8_t i = 0; i < OUTPUT_COMMAND_DIM; i++) {
    outputCommand[i] = 0;
  }

  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    // once someone wakes us up, we assume that they have already written to the
    // speed variable
    xSemaphoreTake(speedMutex, portMAX_DELAY); // wait indefinitely
    xSemaphoreTake(speedLimitMutex, portMAX_DELAY);
    xSemaphoreTake(normalizedMotorCommandsMutex, portMAX_DELAY);
    // first we mix
    // TODO: redo the interface so that it also works with mechanum wheels,
    // espeically, don't use speed.linear or whatever, rather, just use an array
    // so that matrix is generalizable
    outputCommand[0] = MIXER_MATRIX[0][0] * speedLimit.linear * speed.linear +
                       MIXER_MATRIX[0][1] * speedLimit.angular * speed.angular;
    outputCommand[1] = MIXER_MATRIX[1][0] * speedLimit.linear * speed.linear +
                       MIXER_MATRIX[1][1] * speedLimit.angular * speed.angular;

    // now we're doing scaling straight in here

    absOutputCommandMax = fmax(fabs(outputCommand[0]), fabs(outputCommand[1]));

    if (absOutputCommandMax > 1.0f) {
      // rescale everything so that outputCommand is still in between -1 and 1
      outputCommand[0] /= absOutputCommandMax;
      outputCommand[1] /= absOutputCommandMax;
    }

    /*
    mixerLogMessage.timestamp = millis();
    mixerLogMessage.logSource = MIXER;
    sprintf(mixerLogMessage.text,
            "left: %f, right: %f, absOutputCommandMax: %f", outputCommand[0],
            outputCommand[1], absOutputCommandMax);
    xQueueSend(logQueueHandle, &mixerLogMessage, 0);
    */
    xSemaphoreGive(normalizedMotorCommandsMutex);
    xSemaphoreGive(speedMutex);
    xSemaphoreGive(speedLimitMutex);
  }
}
