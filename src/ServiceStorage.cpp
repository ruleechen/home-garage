#include "ServiceStorage.h"

namespace Victor::Components {

  ServiceStorage::ServiceStorage() {
    _filePath = F("/service.json");
    _maxSize = 512;
  }

  void ServiceStorage::_serializeTo(const ServiceModel& model, DynamicJsonDocument& doc) {
    JsonArray item = doc.createNestedArray(F("s"));
    item[0] = model.openSensePin;
    item[1] = model.closeSensePin;
    item[2] = model.openTrueValue;
    item[3] = model.closeTrueValue;
  }

  void ServiceStorage::_deserializeFrom(ServiceModel& model, const DynamicJsonDocument& doc) {
    auto item = doc[F("s")];
    model.openSensePin = item[0];
    model.closeSensePin = item[1];
    model.openTrueValue = item[2];
    model.closeTrueValue = item[3];
  }

  // global
  ServiceStorage serviceStorage;

} // namespace Victor::Components
