#pragma once

// handles receiving inputs from the joystick

#include <Arduino.h>
#include <stdint.h>

// increase or decrease 5% (depending on the variable) of the total range
#define SPEED_UPDATE_STEP 0.05f

// detect on button press, but since the button is normally 1, we detect the
// falling edge
#define BUTTON_INTERRUPT_MODE FALLING

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

void buttonISR(void *arg);

// TODO: update state speed somehow, and write functions for each state that
// should be handled

// this is a task
extern TaskHandle_t debounceThenUpdateTaskHandle;

void debounceThenUpdate(void *arg);
void readAndUpdateJoystick(JoystickState *joystickState);
void handleButtonAfterDebounce(uint8_t buttonPin);
