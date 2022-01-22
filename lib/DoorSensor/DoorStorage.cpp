#include "DoorStorage.h"

namespace Victor::Components {

  DoorStorage::DoorStorage() {
    _filePath = "/door.json";
    _maxSize = 512;
  }

  void DoorStorage::_serializeTo(const DoorSetting& model, DynamicJsonDocument& doc) {
    const JsonArray setting = doc.createNestedArray(F("s"));
    setting[0] = model.openSensorPin;
    setting[1] = model.closedSensorPin;
    setting[2] = model.openTrueValue;
    setting[3] = model.closedTrueValue;
  }

  void DoorStorage::_deserializeFrom(DoorSetting& model, const DynamicJsonDocument& doc) {
    const auto setting = doc[F("s")];
    model.openSensorPin = setting[0];
    model.closedSensorPin = setting[1];
    model.openTrueValue = setting[2];
    model.closedTrueValue = setting[3];
  }

  // global
  DoorStorage doorStorage;

} // namespace Victor::Components
