#include "DoorStorage.h"

namespace Victor::Components {

  DoorStorage::DoorStorage(const char* filePath) {
    _filePath = filePath;
    _maxSize = 512;
  }

  void DoorStorage::_serializeTo(const DoorSetting& model, DynamicJsonDocument& doc) {
    const JsonArray pinArr = doc.createNestedArray(F("pin"));
    pinArr[0] = model.openSensorPin;
    pinArr[1] = model.closedSensorPin;
    pinArr[2] = model.openTrueValue;
    pinArr[3] = model.closedTrueValue;
  }

  void DoorStorage::_deserializeFrom(DoorSetting& model, const DynamicJsonDocument& doc) {
    const auto pinArr = doc[F("pin")];
    model.openSensorPin = pinArr[0];
    model.closedSensorPin = pinArr[1];
    model.openTrueValue = pinArr[2];
    model.closedTrueValue = pinArr[3];
  }

} // namespace Victor::Components
