#pragma once

#include "config.h"

// this variable is normalized to be in between 0 and 1
typedef struct {
  float linear;
  float angular;
} Speed;

extern Speed speed;

typedef struct {
  float linear;
  float angular;
} SpeedLimit;

//
extern SpeedLimit speedLimit;
