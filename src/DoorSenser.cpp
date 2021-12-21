#include "DoorSenser.h"

namespace Victor::Components {

  DoorSenser::DoorSenser(ServiceModel model) {
    _openSenser = new DigitalInput(model.openSenserPin, model.openTrueValue);
    _closedSenser = new DigitalInput(model.closedSenserPin, model.closedTrueValue);
    _lastState = _readState();
  }

  void DoorSenser::loop() {
    auto now = millis();
    if (now - _lastLoop > 1000) {
      _lastLoop = now;
      auto state = _readState();
      if (state != _lastState) {
        _lastState = state;
        if (onStateChange) {
          onStateChange(state);
        }
      }
    }
  }

  DoorState DoorSenser::_readState() {
    DoorState state = _lastState;
    if (_openSenser->getValue()) {
      state = DoorStateOpen;
    } else if (_closedSenser->getValue()) {
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
