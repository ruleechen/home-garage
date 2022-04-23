#include "DoorSensor.h"

#ifndef DOORSENSOR_INTERVAL
#define DOORSENSOR_INTERVAL 250
#endif

#ifndef DOORSENSOR_DEBOUNCE
#define DOORSENSOR_DEBOUNCE 3000
#endif

namespace Victor::Components {

  DoorSensor::DoorSensor(DoorSetting model) {
    _openSensor = new DigitalInput(model.openSensorPin, model.openTrueValue);
    _closedSensor = new DigitalInput(model.closedSensorPin, model.closedTrueValue);
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
      now - _lastLoop > DOORSENSOR_INTERVAL &&
      now - _lastChange > DOORSENSOR_DEBOUNCE
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
      state = DoorStateOpen;
    } else if (_closedSensor->getValue()) {
      state = DoorStateClosed;
    } else {
      if (_lastState == DoorStateOpen) {
        state = DoorStateClosing;
      } else if (_lastState == DoorStateClosed) {
        state = DoorStateOpening;
      }
    }
    return state;
  }

} // namespace Victor::Components
