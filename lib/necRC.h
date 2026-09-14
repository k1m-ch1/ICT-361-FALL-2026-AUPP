#pragma once

#include "config.h"

// if we haven't received a repeat code in 150ms, stop the car
#define REPEAT_CODE_TIMEOUT 150

#define NEC_RC_SPEED_UPDATE_STEP 0.05f

enum AddrMapNEC {
  UP = 0xFF18E7,
  DOWN = 0xFF4AB5,
  LEFT = 0xFF10EF,
  RIGHT = 0xFF5AA5, // TODO: find the address of the OK button
  STAR = 0xFF6897,  // this should decrease speed by 5
  HASH = 0xFFB04F   // this should increase speed by 5
};

void necRCInit();

void necRCTask(void *args);

const char *getAddrMapNECAsString(AddrMapNEC addrMapNEC);
