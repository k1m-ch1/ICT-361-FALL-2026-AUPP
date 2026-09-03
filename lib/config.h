#pragma once

#include <stdint.h>

#define ADC_RESOLUTION 12
#define JOYSTICK_POLLING_RATE 100
#define MOTORS_AMOUNT 4

// we normalize the RC stick to be in between -1 and 1,

// this means we only use
#define STICK_DEADZONE 0.05f

typedef struct {
  uint32_t adcMin;
  uint32_t adcMax;
  uint32_t adcMid;
  float deadzone;
} JoystickConfig;

const JoystickConfig joystickConfig = {
    .adcMin = 0, .adcMax = 4096, .adcMid = 2048, .deadzone = 0.05f};

typedef struct {
  uint8_t in1;
  uint8_t in2;
  uint8_t pwm;
} MotorPins;

const MotorPins motorPins[MOTORS_AMOUNT] = {
    {26, 25, 33}, {32, 27, 14}, {21, 18, 5}, {23, 22, 19}};

typedef struct {
  uint8_t up;
  uint8_t left;
  uint8_t down;
  uint8_t right;
  uint8_t x;
  uint8_t y;
} RemoteControlPins;

const RemoteControlPins remoteControlPins = {16, 2, 15, 4, 35, 34};
