#ifndef DoorModels_h
#define DoorModels_h

#include <Arduino.h>

namespace Victor {

  enum DoorState {
    DOOR_STATE_OPEN = 0,
    DOOR_STATE_CLOSED = 1,
    DOOR_STATE_OPENING = 2,
    DOOR_STATE_CLOSING = 3,
    DOOR_STATE_STOPPED = 4, // stopped not open or closed
  };

  struct DoorSetting {
    // door open
    int8_t doorOpenPin = -1; // (-127 ~ 128)
    uint8_t doorOpenTrueValue = 0; // (0 ~ 256) LOW
    // door closed
    int8_t doorClosedPin = -1; // (-127 ~ 128)
    uint8_t doorClosedTrueValue = 0; // (0 ~ 256) LOW
  };

} // namespace Victor

#endif // DoorModels_h
