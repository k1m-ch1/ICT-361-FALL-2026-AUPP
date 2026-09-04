#pragma once

// handles receiving inputs from the joystick

#include <Arduino.h>
#include <stdint.h>

typedef struct {
  uint32_t x;
  uint32_t y;
} JoystickState;

// buttons should be interrupt based
typedef struct {
  uint32_t up;
  uint32_t left;
  uint32_t down;
  uint32_t right;
} ButtonsState;

// this variable is normalized to be in between 0 and 1
typedef struct {
  float linear;
  float angular;
} Speed;

const Speed speed = {.linear = 0, .angular = 0};

typedef struct {
  float linear;
  float angular;
} SpeedLimit;

//
extern SpeedLimit speedLimit;

//
void buttonISR(void *arg);

// TODO: update state speed somehow, and write functions for each state that
// should be handled

// this is a task
extern TaskHandle_t debounceThenUpdateTaskHandle;

void debounceThenUpdate(void *arg);
void readAndUpdateJoystick(JoystickState *joystickState);
void handleButtonAfterDebounce(uint8_t buttonPin);
