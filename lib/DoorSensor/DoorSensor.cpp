#include "DoorSensor.h"

namespace Victor::Components {

  DoorSensor::DoorSensor(DoorSetting* setting) {
    _openSensor = new DigitalInput(setting->doorOpenPin, setting->doorOpenTrueValue);
    _closedSensor = new DigitalInput(setting->doorClosedPin, setting->doorClosedTrueValue);
    _debounce = new IntervalOver(setting->debounce);
    _currentState = readState();
    // register interrupt
    attachInterrupt(digitalPinToInterrupt(setting->doorOpenPin), _interruptHandler, CHANGE);
    attachInterrupt(digitalPinToInterrupt(setting->doorClosedPin), _interruptHandler, CHANGE);
  }

  void DoorSensor::loop() {
    if (_hasChanges) {
      _hasChanges = false;
      const auto now = millis();
      if (_debounce->isOver(now)) {
        const auto state = readState();
        if (state != _currentState) {
          _currentState = state;
          _debounce->start(now);
          if (onStateChange != nullptr) {
            onStateChange(state);
          }
        }
      }
    }
  }

  CurrentDoorState DoorSensor::readState() {
    auto state = _currentState;
    if (_openSensor->getValue()) {
      state = CURRENT_DOOR_STATE_OPEN;
    } else if (_closedSensor->getValue()) {
      state = CURRENT_DOOR_STATE_CLOSED;
    } else {
      if (_currentState == CURRENT_DOOR_STATE_OPEN) {
        state = CURRENT_DOOR_STATE_CLOSING;
      } else if (_currentState == CURRENT_DOOR_STATE_CLOSED) {
        state = CURRENT_DOOR_STATE_OPENING;
      }
    }
    return state;
  }

  volatile bool DoorSensor::_hasChanges = false;
  void IRAM_ATTR DoorSensor::_interruptHandler() {
    _hasChanges = true;
  }

} // namespace Victor::Components
