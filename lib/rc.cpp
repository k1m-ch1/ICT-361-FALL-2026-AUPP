#pragma once

#include "rc.h"
#include "config.h"
#include "logging.h"
#include "mixer.h"
#include "utils.h"
#include <Arduino.h>
#include <stdint.h>

JoystickState joystickState;
ButtonState buttonState;
ButtonState prevButtonState;

TaskHandle_t debounceThenUpdateTaskHandle;
SemaphoreHandle_t joystickStateMutex;
SemaphoreHandle_t buttonStateMutex;

void rcInit() {
  // initializing pinModes in here
  pinMode(remoteControlPins.x, INPUT);
  pinMode(remoteControlPins.y, INPUT);
  pinMode(remoteControlPins.up, INPUT);
  pinMode(remoteControlPins.left, INPUT);
  pinMode(remoteControlPins.down, INPUT);
  pinMode(remoteControlPins.right, INPUT);

  buttonState = {.up = 0, .left = 0, .down = 0, .right = 0};
  prevButtonState = buttonState;
  joystickState = {
      .x = (joystickConfig.adcDeadzoneMin + joystickConfig.adcDeadzoneMax) / 2,
      .y = (joystickConfig.adcDeadzoneMin + joystickConfig.adcDeadzoneMax) / 2};
  debounceThenUpdateTaskHandle = nullptr;
  joystickStateMutex = xSemaphoreCreateMutex();
  buttonStateMutex = xSemaphoreCreateMutex();

  // automatically create the joystick polling task and the button polling task
  // here.

  xTaskCreate(pollButtonTask, "Polling button Task", 4096, nullptr, 1,
              nullptr); // no need to store the task handle

  xTaskCreate(pollJoystickTask, "Polling joystick Task", 4096, nullptr, 1,
              nullptr); // no need to store the task handle
}

// this is the interrupted based implementation for buttons and button
// debouncing, however, it doesn't really work for some reaason.

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

  LogMessage logMessage;
  logMessage.timestamp = millis();
  logMessage.logSource = RC;
  sprintf(logMessage.text,
          "Received button: %d, linear limit: %f, angular speed limit: %f",
          buttonPin, speedLimit.linear, speedLimit.angular);
  xQueueSend(logQueueHandle, &logMessage, 0);
}

void pollJoystickTask(void *args) {
  LogMessage joystickLogMessage;
  constexpr TickType_t period = pdMS_TO_TICKS(1000 / JOYSTICK_POLLING_RATE);
  TickType_t lastWakeTime = xTaskGetTickCount();

  while (true) {
    // we'll just read as normal, but we'll need to make a mutex lock I guess
    xSemaphoreTake(joystickStateMutex, portMAX_DELAY); // wait indefinitely
    joystickState.x = analogRead(remoteControlPins.x);
    joystickState.y = analogRead(remoteControlPins.y);
    xSemaphoreTake(speedMutex, portMAX_DELAY); // wait indefinitely
    // remap it to the internal variable called speed directly.
    speed.linear = -asymNormalizedMap(
        (float)joystickConfig.adcMin, (float)joystickConfig.adcDeadzoneMin,
        (float)joystickConfig.adcDeadzoneMax, (float)joystickConfig.adcMax,
        (float)joystickState.y);

    speed.angular = asymNormalizedMap(
        (float)joystickConfig.adcMin, (float)joystickConfig.adcDeadzoneMin,
        (float)joystickConfig.adcDeadzoneMax, (float)joystickConfig.adcMax,
        (float)joystickState.x);

    xSemaphoreGive(speedMutex);
    xSemaphoreGive(joystickStateMutex);
    xTaskNotifyGive(mixerTaskHandle);
    vTaskDelayUntil(&lastWakeTime, period);
  }
}

bool detectEdge(uint8_t prevState, uint8_t currentState) {
  if (BUTTON_INTERRUPT_MODE == FALLING) {
    // want to be true when prevState is 1 and current state is 0
    return prevState & (!currentState);
  } else if (BUTTON_POLLING_RATE == RISING) {
    // want to be true when the current state is 0 and current state is 1
    return (!prevState) & currentState;
  } else {
    // it has to be CHANGE otherwise
    return (prevState & (!currentState)) || ((!prevState) & currentState);
  }
}

void pollButtonTask(void *args) {
  constexpr TickType_t period = pdMS_TO_TICKS(1000 / BUTTON_POLLING_RATE);
  TickType_t lastWakeTime = xTaskGetTickCount();

  while (true) {
    // we'll just read as normal, but we'll need to make a mutex lock I guess
    xSemaphoreTake(buttonStateMutex, portMAX_DELAY); // wait indefinitely
    prevButtonState = buttonState;
    // code is a little repetitive, but perhaps more readable?
    buttonState.up = digitalRead(remoteControlPins.up);
    buttonState.down = digitalRead(remoteControlPins.down);
    buttonState.left = digitalRead(remoteControlPins.left);
    buttonState.right = digitalRead(remoteControlPins.right);

    bool upEdgeDetected = detectEdge(prevButtonState.up, buttonState.up);
    bool downEdgeDetected = detectEdge(prevButtonState.down, buttonState.down);
    bool leftEdgeDetected = detectEdge(prevButtonState.left, buttonState.left);
    bool rightEdgeDetected =
        detectEdge(prevButtonState.right, buttonState.right);
    if (upEdgeDetected) {
      // right now, we making polling rate so low that there's no need for
      // debouncing
      handleButtonAfterDebounce(remoteControlPins.up);
    }
    if (downEdgeDetected) {
      handleButtonAfterDebounce(remoteControlPins.down);
    }
    if (leftEdgeDetected) {
      handleButtonAfterDebounce(remoteControlPins.left);
    }
    if (rightEdgeDetected) {
      handleButtonAfterDebounce(remoteControlPins.right);
    }

    xSemaphoreGive(buttonStateMutex);
    vTaskDelayUntil(&lastWakeTime, period);
  }
}
