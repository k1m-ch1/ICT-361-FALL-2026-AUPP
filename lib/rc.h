#pragma once

// handles receiving inputs from the joystick

#include <Arduino.h>
#include <stdint.h>

// increase or decrease 5% (depending on the variable) of the total range
#define SPEED_UPDATE_STEP 0.05f

#define JOYSTICK_POLLING_RATE 125
// if we're polling, we'll poll it so slow that debouncing isn't an issue at
// all.
#define BUTTON_POLLING_RATE 50

// detect on button press, but since the button is normally 1, we detect the
// falling edge
#define BUTTON_INTERRUPT_MODE FALLING

// buttons should be interrupt based
typedef struct {
  uint32_t x;
  uint32_t y;
} JoystickState;

typedef struct {
  uint32_t up;
  uint32_t left;
  uint32_t down;
  uint32_t right;
} ButtonState;

void buttonISR(void *arg);

// TODO: update state speed somehow, and write functions for each state that
// should be handled

extern JoystickState joystickState;
extern ButtonState buttonState;

extern TaskHandle_t debounceThenUpdateTaskHandle;
extern SemaphoreHandle_t joystickStateMutex;
extern SemaphoreHandle_t buttonStateMutex;

void debounceThenUpdate(void *arg);
void handleButtonAfterDebounce(uint8_t buttonPin);
void pollJoystickTask();
void pollButtonTask();
