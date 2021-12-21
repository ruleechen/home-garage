#ifndef DoorSense_h
#define DoorSense_h

#include "ServiceModels.h"

namespace Victor::Components {
  class DoorSense {
   public:
    DoorSense(ServiceModel model);
    void loop();
    // events
    typedef std::function<void(DoorState state)> TStateHandler;
    TStateHandler onStateChange;

   private:
    ServiceModel _setting;
  };

} // namespace Victor::Components

#endif // DoorSense_h
