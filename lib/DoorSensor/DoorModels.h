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
    // door open
    int8_t doorOpenPin = -1;
    uint8_t doorOpenTrueValue = 0;  // LOW
    // door closed
    int8_t doorClosedPin = -1;
    uint8_t doorClosedTrueValue = 0; // LOW
  };

} // namespace Victor

#endif // DoorModels_h
