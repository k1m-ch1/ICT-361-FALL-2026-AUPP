#pragma once

#include "necRC.h"
#include "logging.h"
#include "mixer.h"
#include "nec.h"

const uint8_t getAddrMapNECDigits(AddrMapNEC addrMapNEC) {
  switch (addrMapNEC) {
  case ZERO:
    return 0;
  case ONE:
    return 1;
  case TWO:
    return 2;
  case THREE:
    return 3;
  case FOUR:
    return 4;
  case FIVE:
    return 5;
  case SIX:
    return 6;
  case SEVEN:
    return 7;
  case EIGHT:
    return 8;
  case NINE:
    return 9;
  default:
    return -1;
  }
}

const char *getAddrMapNECAsString(AddrMapNEC addrMapNEC) {
  switch (addrMapNEC) {
  case ZERO:
    return "ZERO";
  case ONE:
    return "ONE";
  case TWO:
    return "TWO";
  case THREE:
    return "THREE";
  case FOUR:
    return "FOUR";
  case FIVE:
    return "FIVE";
  case SIX:
    return "SIX";
  case SEVEN:
    return "SEVEN";
  case EIGHT:
    return "EIGHT";
  case NINE:
    return "NINE";
  case UP:
    return "UP";
  case DOWN:
    return "DOWN";
  case LEFT:
    return "LEFT";
  case RIGHT:
    return "RIGHT";
  case STOP:
    return "STOP";
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
  uint32_t necNumberStored = 0;
  uint8_t necDigitReceived;
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
      continue;
    }

    if (necCommand.repeatFlag) {
      // if we get a repeat flag, do nothing. Essentially, if the previous
      // commands were either up, down, left or right, the motors would be
      // moving like, right now, and we want to do nothing about it. If it was
      // for like, the digits, then do nothing.
      continue;
    }

    // so at this stage, we know that we got a message, and it's not a repeat
    // message.
    if (necCommand.command == UP || necCommand.command == DOWN ||
        necCommand.command == LEFT || necCommand.command == RIGHT ||
        necCommand.command == STOP) {
      // these are commands that will be modifying
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
      case STOP:
        speed.linear = 0.0f;
        speed.angular = 0.0f;
        break;
      }
      xSemaphoreGive(speedMutex);
      prevNecCommand = necCommand;
      xTaskNotifyGive(mixerTaskHandle);
      sprintf(
          necRCLogMessage.text, "Mapped command %X to %s", necCommand.command,
          getAddrMapNECAsString(static_cast<AddrMapNEC>(necCommand.command)));
      xQueueSend(logQueueHandle, &necRCLogMessage, 0);
      continue;
    }

    // if it's STAR or HASH or ZERO, then we're modifying speedLimit
    if (necCommand.command == STAR || necCommand.command == HASH ||
        necCommand.command == ZERO) {
      xSemaphoreTake(speedLimitMutex, portMAX_DELAY);

      if (necCommand.command == STAR) {
        speedLimit.linear -= NEC_RC_SPEED_UPDATE_STEP;
        speedLimit.angular -= NEC_RC_SPEED_UPDATE_STEP;
      } else if (necCommand.command == HASH) {
        speedLimit.linear += NEC_RC_SPEED_UPDATE_STEP;
        speedLimit.angular += NEC_RC_SPEED_UPDATE_STEP;
      } else if (necCommand.command == ZERO) {
        speedLimit.linear = necNumberStored / 100.0f;
        speedLimit.angular = necNumberStored / 100.0f;
        // reset it the stored number afterwards
        necNumberStored = 0;
      }

      speedLimit.linear = max(0.0f, min(speedLimit.linear, 1.0f));
      speedLimit.angular = max(0.0f, min(speedLimit.angular, 1.0f));
      xSemaphoreGive(speedLimitMutex);
      xTaskNotifyGive(mixerTaskHandle);
      prevNecCommand = necCommand;
      sprintf(
          necRCLogMessage.text,
          "mapped command %X to %s. Setting linear: %f, angular: %f",
          necCommand.command,
          getAddrMapNECAsString(static_cast<AddrMapNEC>(necCommand.command)),
          speedLimit.linear, speedLimit.angular);
      xQueueSend(logQueueHandle, &necRCLogMessage, 0);
      continue;
      // only store the validly mapped commands
    }

    // if we're here, then we either received one of the digits, or a completely
    // unknown command

    necDigitReceived =
        getAddrMapNECDigits(static_cast<AddrMapNEC>(necCommand.command));

    if (necDigitReceived > 0) {
      necNumberStored = 10 * necNumberStored + necDigitReceived;
      sprintf(
          necRCLogMessage.text, "mapped command %X to %s. necNumberStored: %lu",
          necCommand.command,
          getAddrMapNECAsString(static_cast<AddrMapNEC>(necCommand.command)),
          necNumberStored);
      xQueueSend(logQueueHandle, &necRCLogMessage, 0);
    } else {
      // if we're here, then we have an unknwon command
      sprintf(necRCLogMessage.text, "Unknown command: %X", necCommand.command);
      xQueueSend(logQueueHandle, &necRCLogMessage, 0);
    }
  }
}
