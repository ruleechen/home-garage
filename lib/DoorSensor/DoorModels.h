#ifndef DoorModels_h
#define DoorModels_h

#include <Arduino.h>

namespace Victor {

  enum DoorState {
    DoorStateOpen = 0,
    DoorStateClosed = 1,
    DoorStateOpening = 2,
    DoorStateClosing = 3,
    DoorStateStopped = 4, // stopped not open or closed
  };

  struct DoorSetting {
    // open
    int8_t openSensorPin = -1;
    uint8_t openTrueValue = 0;  // LOW
    // closed
    int8_t closedSensorPin = -1;
    uint8_t closedTrueValue = 0; // LOW
  };

} // namespace Victor

#endif // DoorModels_h
