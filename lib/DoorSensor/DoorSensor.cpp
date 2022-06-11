#include "DoorSensor.h"

namespace Victor::Components {

  DoorSensor::DoorSensor(DoorSetting model) {
    _openSensor = new DigitalInput(model.doorOpenPin, model.doorOpenTrueValue);
    _closedSensor = new DigitalInput(model.doorClosedPin, model.doorClosedTrueValue);
    _debounce = new IntervalOver(model.debounce);
    _lastState = readState();
    // register interrupt
    attachInterrupt(digitalPinToInterrupt(model.doorOpenPin), _interruptHandler, CHANGE);
    attachInterrupt(digitalPinToInterrupt(model.doorClosedPin), _interruptHandler, CHANGE);
  }

  void DoorSensor::loop() {
    if (_hasChanges) {
      _hasChanges = false;
      const auto now = millis();
      if (_debounce->isOver(now)) {
        const auto state = readState();
        if (state != _lastState) {
          _lastState = state;
          _debounce->start(now);
          if (onStateChange != nullptr) {
            onStateChange(state);
          }
        }
      }
    }
  }

  DoorState DoorSensor::readState() {
    auto state = _lastState;
    if (_openSensor->getValue()) {
      state = DOOR_STATE_OPEN;
    } else if (_closedSensor->getValue()) {
      state = DOOR_STATE_CLOSED;
    } else {
      if (_lastState == DOOR_STATE_OPEN) {
        state = DOOR_STATE_CLOSING;
      } else if (_lastState == DOOR_STATE_CLOSED) {
        state = DOOR_STATE_OPENING;
      }
    }
    return state;
  }

  volatile bool DoorSensor::_hasChanges = false;
  void IRAM_ATTR DoorSensor::_interruptHandler() {
    _hasChanges = true;
  }

} // namespace Victor::Components
