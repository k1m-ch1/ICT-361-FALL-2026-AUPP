#include "logging.h"
#include "nec.h"
#include "necRC.h"
#include "mixer.h"
#include "motors.h"

void setup(){
  motorsInit();
  loggingInit();
  necInit();
  necRCInit();
  mixerInit();
}

void loop(){
}
