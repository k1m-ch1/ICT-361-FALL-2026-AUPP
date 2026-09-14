#pragma once

#include "necRC.h"
#include "config.h"
#include "logging.h"
#include "mixer.h"
#include "nec.h"

const char *getAddrMapNECAsString(AddrMapNEC addrMapNEC) {
  switch (addrMapNEC) {
  case UP:
    return "UP";
  case DOWN:
    return "DOWN";
  case LEFT:
    return "LEFT";
  case RIGHT:
    return "RIGHT";
  case STAR:
    return "STAR";
  case HASH:
    return "HASH";
  }
}

void necRCInit() {
  xTaskCreate(necRCTask, "NEC RC task", 4096, nullptr, 1, nullptr);
}

void necRCTask(void *args) {
  NECCommand prevNecCommand;
  NECCommand necCommand;
  LogMessage necRCLogMessage;
  necRCLogMessage.logSource = RC;
  BaseType_t result;
  while (true) {
    result = xQueueReceive(necCommandQueueHandle, &necCommand,
                           pdMS_TO_TICKS(REPEAT_CODE_TIMEOUT));
    necRCLogMessage.timestamp = millis();
    necRCLogMessage.logSource = NEC_RC;
    if (result == pdFAIL) {
      // if timeout has been reached, force stop motors
      xSemaphoreTake(speedMutex, portMAX_DELAY);
      // remap it to the internal variable called speed directly.
      speed.linear = 0.0f;
      speed.angular = 0.0f;
      xSemaphoreGive(speedMutex);
      xTaskNotifyGive(mixerTaskHandle);
      /*
      sprintf(necRCLogMessage.text,
              "reached a REPEAT_CODE_TIMEOUT. Force stopping the robot.");
      xQueueSend(logQueueHandle, &necRCLogMessage, 0);
      */
      continue;
    }

    // check whether the previous NEC commands are the up, down, left, right
    // command.
    if (necCommand.repeatFlag &&
        (prevNecCommand.command == UP || prevNecCommand.command == DOWN ||
         prevNecCommand.command == LEFT || prevNecCommand.command == RIGHT)) {
      // so, if the previous command was the up, down, left, right, button, and
      // we got a repeat flag, then assume that the robot is still going up,
      // down, left, right respectively, and do nothing

      sprintf(necRCLogMessage.text, "Got repeat flag. previous command was %s",
              getAddrMapNECAsString(
                  static_cast<AddrMapNEC>(prevNecCommand.command)));
      xQueueSend(logQueueHandle, &necRCLogMessage, 0);

      continue;
    }

    // so at this stage, we know that we got a message, and it's not a repeat
    // message.
    if (necCommand.command == UP || necCommand.command == DOWN ||
        necCommand.command == LEFT || necCommand.command == RIGHT) {
      // TODO: add a stop command too
      // so we first grab the speed mutex because we'll be modifying that
      xSemaphoreTake(speedMutex, portMAX_DELAY);
      switch (necCommand.command) {
      case UP:
        speed.linear = 1.0f;
        speed.angular = 0.0f;
        break;
      case DOWN:
        speed.linear = -1.0f;
        speed.angular = 0.0f;
        break;
      case LEFT:
        speed.linear = 0.0f;
        speed.angular = 1.0f;
        break;
      case RIGHT:
        speed.linear = 0.0f;
        speed.angular = -1.0f;
        break;
      }
      xSemaphoreGive(speedMutex);
      prevNecCommand = necCommand;
      xTaskNotifyGive(mixerTaskHandle);
      sprintf(
          necRCLogMessage.text, "mapped command %X to %s", necCommand.command,
          getAddrMapNECAsString(static_cast<AddrMapNEC>(necCommand.command)));
    } else if (necCommand.command == STAR || necCommand.command == HASH) {
      xSemaphoreTake(speedLimitMutex, portMAX_DELAY);
      if (necCommand.command == STAR) {
        speedLimit.linear -= NEC_RC_SPEED_UPDATE_STEP;
        speedLimit.angular -= NEC_RC_SPEED_UPDATE_STEP;
      } else if (necCommand.command == HASH) {
        speedLimit.linear += NEC_RC_SPEED_UPDATE_STEP;
        speedLimit.angular += NEC_RC_SPEED_UPDATE_STEP;
      }
      speedLimit.linear = max(0.0f, min(speedLimit.linear, 1.0f));
      speedLimit.angular = max(0.0f, min(speedLimit.angular, 1.0f));
      xSemaphoreGive(speedLimitMutex);
      prevNecCommand = necCommand;
      xTaskNotifyGive(mixerTaskHandle);
      // only store the validly mapped commands
    } else {
      sprintf(necRCLogMessage.text, "unknown command %X", necCommand.command);
    }
    xQueueSend(logQueueHandle, &necRCLogMessage, 0);
  }
}
