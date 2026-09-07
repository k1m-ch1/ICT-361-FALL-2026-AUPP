#include "config.h"
#include "logging.h"
#include "mixer.h"
#include "rc.h"
#include "motors.h"

void setup(){
  loggingInit();
  mixerInit();
  rcInit();
  motorsInit();
}

void loop(){
}
