#include "DoorSensor.h"

namespace Victor::Components {

  DoorSensor::DoorSensor(DoorSetting model) {
    _openSensor = new DigitalInput(model.doorOpenPin, model.doorOpenTrueValue);
    _closedSensor = new DigitalInput(model.doorClosedPin, model.doorClosedTrueValue);
    _lastState = readState();
  }

  DoorSensor::~DoorSensor() {
    delete _openSensor;
    _openSensor = nullptr;
    delete _closedSensor;
    _closedSensor = nullptr;
  }

  void DoorSensor::loop() {
    const auto now = millis();
    if (
      now - _lastLoop > VICTOR_DOOR_SENSOR_INTERVAL &&
      now - _lastChange > VICTOR_DOOR_SENSOR_DEBOUNCE
    ) {
      _lastLoop = now;
      const auto state = readState();
      if (state != _lastState) {
        _lastState = state;
        _lastChange = now;
        if (onStateChange != nullptr) {
          onStateChange(state);
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

} // namespace Victor::Components
