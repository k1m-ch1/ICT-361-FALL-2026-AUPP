#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define EDGE_INFO_QUEUE_SIZE 32

#define STATE_MACHINE_FREQ 8000

#define IR_RECEIVER_PIN 23

// if we're in some state in the state machine, and it has been 35ms since we
// received another edge (can set it in the xQueueReceive)

#define TO_IDLE_TIMEOUT 35

enum NECState {
  IDLE,
  ACG_PASSED,
  PAUSE_PASSED,
  LOW_OF_BIT_RECOGNIZED,
  STATES_COUNT
};

extern const char *statesAsString[STATES_COUNT];

typedef struct {
  uint32_t mean;
  uint32_t epsilon;
} AcceptedRanges;
// this will be in microseconds

typedef struct {
  uint32_t timestamp;
  uint8_t type;
} EdgeInfo;

extern QueueHandle_t edgeInfoQueueHandle;

const AcceptedRanges acgRange = {.mean = 9000, .epsilon = 500};

const AcceptedRanges offPeriodRange = {.mean = 4500, .epsilon = 200};

const AcceptedRanges lowOfBitRange = {.mean = 562, .epsilon = 200};

const AcceptedRanges highOfBitRange[2] = {{.mean = 562, .epsilon = 200},
                                          {.mean = 1687, .epsilon = 200}};

void necInit();

void necEdgeISR();

void necDecoderTask(void *args);
