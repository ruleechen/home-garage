#ifndef DoorSensor_h
#define DoorSensor_h

#include <Arduino.h>
#include <DigitalInput.h>
#include <Timer/IntervalOver.h>
#include "DoorModels.h"

namespace Victor::Components {
  class DoorSensor {
   public:
    DoorSensor(DoorSetting setting);
    void loop();
    DoorState readState();
    // events
    typedef std::function<void(const DoorState state)> TStateHandler;
    TStateHandler onStateChange = nullptr;

   private:
    DigitalInput* _openSensor = nullptr;
    DigitalInput* _closedSensor = nullptr;
    IntervalOver* _debounce = nullptr;
    DoorState _lastState = DOOR_STATE_STOPPED;
    // interrupt
    volatile static bool _hasChanges;
    static void IRAM_ATTR _interruptHandler();
  };

} // namespace Victor::Components

#endif // DoorSensor_h
