#pragma once

// handles receiving inputs from the joystick

#include <stdint.h>

typedef struct {
  uint32_t x;
  uint32_t y;
} JoyStickState;

// buttons should be interrupt based
typedef struct {
  uint32_t up;
  uint32_t left;
  uint32_t down;
  uint32_t right;
} ButtonsState;
