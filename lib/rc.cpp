#pragma once

#include "rc.h"
#include "config.h"
#include <Arduino.h>
#include <stdint.h>

TaskHandle_t debounceThenUpdateTaskHandle = nullptr;

// SpeedLimit speedLimit = {.linear = 0.5f, .angular = 0.5f};

void buttonISR(void *arg) {
  // we're expecting the arg to be the button pin
  uint32_t buttonPin = (uint32_t)arg;

  BaseType_t higherPriorityTaskWoken = pdFALSE;

  xTaskNotifyFromISR(debounceThenUpdateTaskHandle, buttonPin,
                     eSetValueWithOverwrite, &higherPriorityTaskWoken);

  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

// TODO: update state speed somehow, and write functions for each state that
// should be handled

void debounceThenUpdate(void *arg) {
  while (1) {
    // we're expecting arg to contain the button pin
    uint32_t buttonPin = (uint32_t)arg;

    xTaskNotifyWait(0,            // bits to clear on entry
                    0,            // bits to clear on exit
                    &buttonPin,   // where to put notification value
                    portMAX_DELAY // wait forever
    );

    // button is now BUTTON_UP, BUTTON_DOWN, etc.

    delay(joystickConfig.debounceDelayMs);
    handleButtonAfterDebounce(buttonPin);
  }
}

void handleButtonAfterDebounce(uint8_t buttonPin) {
  bool buttonStateAfterDebounce = digitalRead(buttonPin);
  if (buttonStateAfterDebounce == 1) {
    // since the buttons are normally 1, we'll check that if it's still 1, then
    // we ignore it
    return;
  }

  Serial.printf("Received button: %d, timestamp: %lu\r\n", buttonPin, millis());

  /*
  if (buttonPin == remoteControlPins.up) {

  }
  */
}

void readAndUpdateJoystick(JoystickState *joystickState) {}
