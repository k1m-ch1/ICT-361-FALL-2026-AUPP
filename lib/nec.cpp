#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "logging.h"

#include "config.h"
#include "nec.h"

const char *statesAsString[STATES_COUNT] = {
    "IDLE", "ACG_PASSED", "PAUSE_PASSED", "LOW_OF_BIT_RECOGNIZED"};

QueueHandle_t edgeInfoQueueHandle;
QueueHandle_t necCommandQueueHandle;

void necInit() {
  // just something to get it started
  necCommandQueueHandle =
      xQueueCreate(NEC_COMMAND_QUEUE_SIZE, sizeof(NECCommand));

  pinMode(IR_RECEIVER_PIN, INPUT);
  edgeInfoQueueHandle = xQueueCreate(EDGE_INFO_QUEUE_SIZE, sizeof(EdgeInfo));
  attachInterrupt(digitalPinToInterrupt(IR_RECEIVER_PIN), necEdgeISR, CHANGE);
  xTaskCreate(necDecoderTask, "NEC decoder task", 4096, nullptr, 1, nullptr);
}

void necEdgeISR() {
  EdgeInfo edgeInfo;
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  if (digitalRead(IR_RECEIVER_PIN)) {
    // if it's 1, assume we're fast enough to detect that it's a rising edge
    edgeInfo.type = RISING;
  } else {
    edgeInfo.type = FALLING;
  }
  edgeInfo.timestamp = micros();
  xQueueSendFromISR(edgeInfoQueueHandle, &edgeInfo, &higherPriorityTaskWoken);

  if (higherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

void necDecoderTask(void *args) {
  EdgeInfo prevEdge = {.timestamp = micros(), .type = FALLING};
  EdgeInfo currentEdge = {.timestamp = micros(), .type = RISING};
  NECState currentState = IDLE;
  uint32_t pulseTime = 0;
  EdgeInfo edgeInfo;
  BaseType_t result;
  LogMessage necLogMessage;
  NECCommand necCommand;
  uint8_t bitCount = 0;
  uint32_t receivedBit = 0;
  uint32_t now = millis();
  necLogMessage.logSource = NEC;
  while (true) {
    result = xQueueReceive(edgeInfoQueueHandle, &edgeInfo,
                           pdMS_TO_TICKS(TO_IDLE_TIMEOUT));
    now = millis();
    if (result == pdFAIL) {
      // if we've reached the timeout
      if (currentState == LOW_OF_BIT_RECOGNIZED && bitCount == 0) {
        // this is probably the repeat code

        // send the repeat code to the queue
        necCommand.command = 0;
        necCommand.repeatFlag = 1;
        necCommand.timestamp = now;
        xQueueSend(necCommandQueueHandle, &necCommand, 0);

        // now send a log message
        necLogMessage.timestamp = now;
        sprintf(necLogMessage.text, "This might be a REPEAT code");
        xQueueSend(logQueueHandle, &necLogMessage, 0);
      }
      bitCount = 0;
      receivedBit = 0;
      currentState = IDLE;
      continue;
    }

    prevEdge = currentEdge;
    currentEdge = edgeInfo;
    pulseTime = currentEdge.timestamp - prevEdge.timestamp;

    // let's now build the state machine
    switch (currentState) {
    case IDLE:
      bitCount = 0;
      receivedBit = 0;
      if ((currentEdge.type == RISING) &&
          (pulseTime <= acgRange.mean + acgRange.epsilon) &&
          (pulseTime >= acgRange.mean - acgRange.epsilon)) {
        currentState = ACG_PASSED;
      }
      break;
    case ACG_PASSED:
      if (!((currentEdge.type == FALLING) &&
            (pulseTime <= offPeriodRange.mean + offPeriodRange.epsilon) &&
            (pulseTime >= offPeriodRange.mean - offPeriodRange.epsilon))) {
        currentState = IDLE;
      }
      currentState = PAUSE_PASSED;
      break;
    case PAUSE_PASSED:
      if (!((currentEdge.type == RISING) &&
            (pulseTime <= lowOfBitRange.mean + lowOfBitRange.epsilon) &&
            (pulseTime >= lowOfBitRange.mean - lowOfBitRange.epsilon))) {
        currentState = IDLE;
      }
      if (bitCount == 32) {
        // send the complete frame to the command queue
        necCommand.command = receivedBit;
        necCommand.repeatFlag = 0;
        necCommand.timestamp = now;
        xQueueSend(necCommandQueueHandle, &necCommand, 0);

        necLogMessage.timestamp = now;
        sprintf(necLogMessage.text, "Got a complete frame: %X", receivedBit,
                pulseTime);
        xQueueSend(logQueueHandle, &necLogMessage, 0);
        // after we get a complete frame, just go back to idle
        currentState = IDLE;
        bitCount = 0;
        receivedBit = 0;
      }
      currentState = LOW_OF_BIT_RECOGNIZED;
      break;
    case LOW_OF_BIT_RECOGNIZED:
      // branch into many things...
      if (currentEdge.type != FALLING) {
        currentState = IDLE;
        break;
      }

      if (pulseTime <= highOfBitRange[0].mean + highOfBitRange[0].epsilon &&
          pulseTime >= highOfBitRange[0].mean - highOfBitRange[0].epsilon) {
        // if it's within the highOfBitRange[0], then we just move to the pause
        // passed, while storing the bit as 0
        bitCount += 1;
        receivedBit = (receivedBit << 1) + 0b0;

      } else if (pulseTime <=
                     highOfBitRange[1].mean + highOfBitRange[1].epsilon &&
                 pulseTime >=
                     highOfBitRange[1].mean - highOfBitRange[1].epsilon) {
        bitCount += 1;
        receivedBit = (receivedBit << 1) + 0b1;
      } else {
        currentState = IDLE;
        break;
      }

      // NOTE: there's an additional case where if the pulseTime is indefinitely
      // high, we're sending a repeat code, but this will be handled by the
      // queueTimeout

      /*
      necLogMessage.timestamp = now;
      sprintf(necLogMessage.text, "bitCount: %d, receivedBit: %lu, got 1",
                bitCount, receivedBit, pulseTime);

      xQueueSend(logQueueHandle, &necLogMessage, 0);
      */
      currentState = PAUSE_PASSED;
      break;
    }
  }
}
