#pragma once

#include "rc.h"
#include "config.h"
#include "logging.h"
#include "mixer.h"
#include <Arduino.h>
#include <stdint.h>

JoystickState joystickState;
ButtonState buttonState;
ButtonState prevButtonState;

TaskHandle_t debounceThenUpdateTaskHandle;
SemaphoreHandle_t joystickStateMutex;
SemaphoreHandle_t buttonStateMutex;

void rcInit() {
  buttonState = {.up = 0, .left = 0, .down = 0, .right = 0};
  joystickState = {.x = joystickConfig.adcMid, .y = joystickConfig.adcMid};
  debounceThenUpdateTaskHandle = nullptr;
  joystickStateMutex = xSemaphoreCreateMutex();
  buttonStateMutex = xSemaphoreCreateMutex();
}

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

  LogMessage logMessage;
  logMessage.timestamp = millis();
  logMessage.logSource = RC;
  sprintf(logMessage.text, "Received button: %d, timestamp: %lu\r\n", buttonPin,
          millis());
  xQueueSend(logQueueHandle, &logMessage, 0);

  // kinda lengthy just to get a log message out, but maybe it's managable...

  // sending it to the queue
  switch (buttonPin) {
  case remoteControlPins.up:
    speedLimit.linear += SPEED_UPDATE_STEP;
    break;
  case remoteControlPins.down:
    speedLimit.linear -= SPEED_UPDATE_STEP;
    break;
  case remoteControlPins.left:
    speedLimit.angular -= SPEED_UPDATE_STEP;
    break;
  case remoteControlPins.right:
    speedLimit.angular += SPEED_UPDATE_STEP;
    break;
  }

  // there's no arguing that speedLimit is noramlized to between 0 and 1
  speedLimit.linear = max(0.0f, min(speedLimit.linear, 1.0f));
  speedLimit.angular = max(0.0f, min(speedLimit.angular, 1.0f));
  /*
  Serial.print("linear: ");
  Serial.print(speedLimit.linear);
  Serial.print(", angular: ");
  Serial.println(speedLimit.angular);
  */
}

void pollJoystickTask() {
  const TickType_t period = pdMS_TO_TICKS((1 / JOYSTICK_POLLING_RATE));
  TickType_t lastWakeTime = xTaskGetTickCount();

  while (true) {
    // we'll just read as normal, but we'll need to make a mutex lock I guess
    xSemaphoreTake(joystickStateMutex, portMAX_DELAY); // wait indefinitely
    joystickState.x = analogRead(remoteControlPins.x);
    joystickState.y = analogRead(remoteControlPins.x);
    xSemaphoreGive(joystickStateMutex);
    vTaskDelayUntil(&lastWakeTime, period);
  }
}

void pollButtonTask() {

  const TickType_t period = pdMS_TO_TICKS((1 / BUTTON_POLLING_RATE));
  TickType_t lastWakeTime = xTaskGetTickCount();

  while (true) {
    // we'll just read as normal, but we'll need to make a mutex lock I guess
    xSemaphoreTake(joystickStateMutex, portMAX_DELAY); // wait indefinitely
    buttonState.up = digitalRead(remoteControlPins.up);
    buttonState.down = digitalRead(remoteControlPins.down);
    buttonState.left = digitalRead(remoteControlPins.left);
    buttonState.right = digitalRead(remoteControlPins.right);
    xSemaphoreGive(joystickStateMutex);
    vTaskDelayUntil(&lastWakeTime, period);
  }
}
