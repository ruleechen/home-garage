#ifndef DoorSenser_h
#define DoorSenser_h

#include <DigitalInput.h>
#include "DoorModels.h"

namespace Victor::Components {
  class DoorSenser {
   public:
    DoorSenser(DoorSetting model);
    ~DoorSenser();
    void loop();
    DoorState readState();
    // events
    typedef std::function<void(const DoorState state)> TStateHandler;
    TStateHandler onStateChange = nullptr;

   private:
    DigitalInput* _openSenser = nullptr;
    DigitalInput* _closedSenser = nullptr;
    DoorState _lastState = DoorStateStopped;
    unsigned long _lastLoop = 0;
    unsigned long _lastChange = 0;
  };

} // namespace Victor::Components

#endif // DoorSenser_h
