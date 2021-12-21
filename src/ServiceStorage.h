#ifndef ServiceStorage_h
#define ServiceStorage_h

#include <FileStorage.h>
#include "ServiceModels.h"

namespace Victor::Components {
  class ServiceStorage : public FileStorage<ServiceModel> {
   public:
    ServiceStorage();

   protected:
    void _serializeTo(const ServiceModel& model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(ServiceModel& model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern ServiceStorage serviceStorage;

} // namespace Victor::Components

#endif // ServiceStorage_h
