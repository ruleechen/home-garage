#ifndef ServiceModels_h
#define ServiceModels_h

#include <map>
#include <Arduino.h>

namespace Victor {

  enum DoorState {
    DoorStateOpen = 0,
    DoorStateClosed = 1,
    DoorStateOpening = 2,
    DoorStateClosing = 3,
    DoorStateStopped = 4,
    DoorStateUnknown = 255,
  };

  struct ServiceModel {
    int8_t openSenserPin = -1;
    int8_t closedSenserPin = -1;
    uint8_t openTrueValue = 0;  // LOW
    uint8_t closedTrueValue = 0; // LOW
  };

} // namespace Victor

#endif // ServiceModels_h
