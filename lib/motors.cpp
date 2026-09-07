#pragma once

#include "motors.h"
#include "config.h"
#include "motors.h"
#include "rc.h"
#include "utils.h"
#include <Arduino.h>
#include <math.h>

void motorsInit() {

  /*
  const MotorPins motorPins[MOTORS_AMOUNT] = {
      {.in1 = 26, .in2 = 25, .pwm = 33},
      {.in1 = 32, .in2 = 27, .pwm = 14},
      {.in1 = 21, .in2 = 18, .pwm = 5},
      {.in1 = 23, .in2 = 22, .pwm = 19}};

  */
  // initialize the pinModes
  for (uint8_t i = 0; i < MOTORS_AMOUNT; i++) {
    pinMode(motorsPins[i].in1, OUTPUT);
    pinMode(motorsPins[i].in2, OUTPUT);
    // now initialize the PWM pins
    ledcAttach(motorsPins[i].pwm, motorConfig.freq, motorConfig.resolution);
  }
}

void motorWrite(MotorPins motorPins, float normalizedMotorCommand) {
  uint32_t duty = 0;
  uint32_t scalingFactor = 1 << motorConfig.resolution;
  // duty isn't normalized, and is scaled by the resolution that we choose to
  // initialize lecdAttach. For instance, if we choose 8, it will be from 0 to
  // 256

  // so we assume that the deadzone is symmetric

  float magnitude = fabsf(normalizedMotorCommand);
  duty = mapFloat(magnitude, 0, 1, motorConfig.deadzone * scalingFactor,
                  scalingFactor);
  if (normalizedMotorCommand > 0) {
    digitalWrite(motorPins.in1, HIGH);
    digitalWrite(motorPins.in2, LOW);
  } else if (normalizedMotorCommand < 0) {
    digitalWrite(motorPins.in1, LOW);
    digitalWrite(motorPins.in2, HIGH);
  } else {
    // if it's 0
    digitalWrite(motorPins.in1, LOW);
    digitalWrite(motorPins.in2, LOW);
  }

  ledcWrite(motorPins.pwm, duty);
}
