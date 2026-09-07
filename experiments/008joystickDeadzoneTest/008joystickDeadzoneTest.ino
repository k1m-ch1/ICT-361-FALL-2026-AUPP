#include "config.h"
#include "logging.h"
#include "mixer.h"
#include "rc.h"

void setup(){
  loggingInit();
  mixerInit();
  rcInit();
}

void loop(){
}
