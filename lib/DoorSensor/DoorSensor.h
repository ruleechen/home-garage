#ifndef DoorSensor_h
#define DoorSensor_h

#include <DigitalInput.h>
#include <Timer/IntervalOver.h>
#include "DoorModels.h"

#ifndef VICTOR_DOOR_SENSOR_INTERVAL
#define VICTOR_DOOR_SENSOR_INTERVAL 250
#endif

namespace Victor::Components {
  class DoorSensor {
   public:
    DoorSensor(DoorSetting model);
    void loop();
    DoorState readState();
    // events
    typedef std::function<void(const DoorState state)> TStateHandler;
    TStateHandler onStateChange = nullptr;

   private:
    DigitalInput* _openSensor = nullptr;
    DigitalInput* _closedSensor = nullptr;
    IntervalOver* _interval = nullptr;
    IntervalOver* _debounce = nullptr;
    DoorState _lastState = DOOR_STATE_STOPPED;
  };

} // namespace Victor::Components

#endif // DoorSensor_h
