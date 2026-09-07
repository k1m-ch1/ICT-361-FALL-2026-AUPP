#pragma once

#include "config.h"
#include <stdint.h>

void motorsInit();

void motorWrite(MotorPins motorPins, float normalizedMotorCommand);
