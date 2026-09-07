#pragma once

#include "config.h"
#include <Arduino.h>
#include <stdint.h>

// this refers to our linear velocity command and our angular velocity command
#define CONTROL_COMMAND_DIM 2

// this refers to our left motor command and right motor velocity command.
#define OUTPUT_COMMAND_DIM 2

// mathematically, this is our mixer matrix M
const int8_t MIXER_MATRIX[CONTROL_COMMAND_DIM][OUTPUT_COMMAND_DIM] = {{1, -1},
                                                                      {1, 1}};

// config is:
// M1 M3
// M2 M4
//
// in our case:
//
// M0 M2
// M1 M3

// this is our sign matrix S
const int8_t SIGN_MATRIX[MOTORS_AMOUNT][OUTPUT_COMMAND_DIM] = {
    {-1, 0}, {-1, 0}, {0, 1}, {0, 1}};

// we now need to store our motor commands:

extern SemaphoreHandle_t normalizedMotorCommandsMutex;
extern float normalizedMotorCommands[MOTORS_AMOUNT];

// this variable is normalized to be in between 0 and 1
typedef struct {
  float linear;
  float angular;
} Speed;

extern Speed speed;
extern SemaphoreHandle_t speedMutex;

typedef struct {
  float linear;
  float angular;
} SpeedLimit;

extern SpeedLimit speedLimit;
extern SemaphoreHandle_t speedLimitMutex;

// this need a mixerTaskHandle because it's notification based
extern TaskHandle_t mixerTaskHandle;
void mixerInit();
void mixerTask(void *args);
