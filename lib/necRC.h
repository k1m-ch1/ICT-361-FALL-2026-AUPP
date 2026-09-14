#pragma once

#include "config.h"

// if we haven't received a repeat code in 150ms, stop the car
#define REPEAT_CODE_TIMEOUT 150

#define NEC_RC_SPEED_UPDATE_STEP 0.05f

enum AddrMapNEC {
  ZERO = 0xFF9867,
  ONE = 0xFFA25D,
  TWO = 0xFF629D,
  THREE = 0xFFE21D,
  FOUR = 0xFF22DD,
  FIVE = 0xFF02FD,
  SIX = 0xFFC23D,
  SEVEN = 0xFFE01F,
  EIGHT = 0xFFA857,
  NINE = 0xFF906F,
  UP = 0xFF18E7,
  DOWN = 0xFF4AB5,
  LEFT = 0xFF10EF,
  RIGHT = 0xFF5AA5,
  STOP = 0xFF38C7,
  STAR = 0xFF6897, // this should decrease speed by 5
  HASH = 0xFFB04F  // this should increase speed by 5
};

const uint8_t getAddrMapNECDigits(AddrMapNEC addrMapNEC);

void necRCInit();

void necRCTask(void *args);

const char *getAddrMapNECAsString(AddrMapNEC addrMapNEC);
