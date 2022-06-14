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

  enum DoorCommand {
    DOOR_COMMAND_OPEN = 0,
    DOOR_COMMAND_CLOSE = 1,
    DOOR_COMMAND_STOP = 2,
  };

  struct DoorSetting {
    // door open sensor input
    // 0~128 = gpio
    //    -1 = disabled
    int8_t doorOpenPin = -1; // (-127 ~ 128)
    // 0 = LOW
    // 1 = HIGH
    uint8_t doorOpenTrueValue = 0; // (0 ~ 256) LOW

    // door closed sensor input
    // 0~128 = gpio
    //    -1 = disabled
    int8_t doorClosedPin = -1; // (-127 ~ 128)
    // 0 = LOW
    // 1 = HIGH
    uint8_t doorClosedTrueValue = 0; // (0 ~ 256) LOW

    // ms debounce time to avoid fast changes
    uint16_t debounce = 0; // (0 ~ 65535)

    // auto stop
    //         0 = disabled
    //         1 = enabled
    // 2 ~ 65535 = ms time delay before emit stop command
    uint16_t autoStop = 0; // (0 ~ 65535)
  };

} // namespace Victor

#endif // DoorModels_h
