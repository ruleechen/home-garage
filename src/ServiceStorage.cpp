#include "ServiceStorage.h"

namespace Victor::Components {

  ServiceStorage::ServiceStorage() {
    _filePath = F("/service.json");
    _maxSize = 512;
  }

  void ServiceStorage::_serializeTo(const ServiceModel& model, DynamicJsonDocument& doc) {
    JsonArray item = doc.createNestedArray(F("s"));
    item[0] = model.openSenserPin;
    item[1] = model.closedSenserPin;
    item[2] = model.openTrueValue;
    item[3] = model.closedTrueValue;
  }

  void ServiceStorage::_deserializeFrom(ServiceModel& model, const DynamicJsonDocument& doc) {
    auto item = doc[F("s")];
    model.openSenserPin = item[0];
    model.closedSenserPin = item[1];
    model.openTrueValue = item[2];
    model.closedTrueValue = item[3];
  }

  // global
  ServiceStorage serviceStorage;

} // namespace Victor::Components
