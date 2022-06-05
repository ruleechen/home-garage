#ifndef DoorSensor_h
#define DoorSensor_h

#include <DigitalInput.h>
#include "DoorModels.h"

#ifndef VICTOR_DOOR_SENSOR_INTERVAL
#define VICTOR_DOOR_SENSOR_INTERVAL 250
#endif

#ifndef VICTOR_DOOR_SENSOR_DEBOUNCE
#define VICTOR_DOOR_SENSOR_DEBOUNCE 3000
#endif

namespace Victor::Components {
  class DoorSensor {
   public:
    DoorSensor(DoorSetting model);
    ~DoorSensor();
    void loop();
    DoorState readState();
    // events
    typedef std::function<void(const DoorState state)> TStateHandler;
    TStateHandler onStateChange = nullptr;

   private:
    DigitalInput* _openSensor = nullptr;
    DigitalInput* _closedSensor = nullptr;
    DoorState _lastState = DoorStateStopped;
    unsigned long _lastLoop = 0;
    unsigned long _lastChange = 0;
  };

} // namespace Victor::Components

#endif // DoorSensor_h
