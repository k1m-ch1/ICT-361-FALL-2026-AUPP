#pragma once

#include <stdint.h>

#define ADC_RESOLUTION 12
#define JOYSTICK_POLLING_RATE 100
#define MOTORS_AMOUNT 4

typedef struct {
  uint8_t up;
  uint8_t left;
  uint8_t down;
  uint8_t right;
  uint8_t x;
  uint8_t y;
} RemoteControlPins;

const RemoteControlPins remoteControlPins = {
    .up = 16, .left = 2, .down = 15, .right = 4, .x = 35, .y = 34};

// we normalize the joystick to be in between -1 and 1,

typedef struct {
  uint32_t adcMin;
  uint32_t adcMax;
  uint32_t adcMid;
  float deadzone;
  uint32_t debounceDelayMs;
} JoystickConfig;

const JoystickConfig joystickConfig = {.adcMin = 0,
                                       .adcMax = 4096,
                                       .adcMid = 2048,
                                       .deadzone = 0.0f,
                                       .debounceDelayMs = 0};

// TODO: find deadzone for joystick

typedef struct {
  uint8_t in1;
  uint8_t in2;
  uint8_t pwm;
} MotorPins;

const MotorPins motorPins[MOTORS_AMOUNT] = {{.in1 = 26, .in2 = 25, .pwm = 33},
                                            {.in1 = 32, .in2 = 27, .pwm = 14},
                                            {.in1 = 21, .in2 = 18, .pwm = 5},
                                            {.in1 = 23, .in2 = 22, .pwm = 19}};

typedef struct {
  uint32_t freq;
  uint8_t resolution;
  float deadzone;
} MotorConfig;

const MotorConfig motorConfig = {
    .freq = 20000, .resolution = 8, .deadzone = 0.0f};

// TODO: find motor deadzone in terms of duty cycle
