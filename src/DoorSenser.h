#ifndef DoorSenser_h
#define DoorSenser_h

#include <DigitalInput.h>
#include "ServiceModels.h"

namespace Victor::Components {
  class DoorSenser {
   public:
    DoorSenser(ServiceModel model);
    void loop();
    // events
    typedef std::function<void(DoorState state)> TStateHandler;
    TStateHandler onStateChange;

   private:
    DigitalInput* _openSenser;
    DigitalInput* _closedSenser;
    DoorState _lastState = DoorStateUnknown;
    unsigned long _lastLoop;
    DoorState _readState();
  };

} // namespace Victor::Components

#endif // DoorSenser_h
