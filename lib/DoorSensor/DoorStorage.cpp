#include "DoorStorage.h"

namespace Victor::Components {

  DoorStorage::DoorStorage(const char* filePath) {
    _filePath = filePath;
    _maxSize = 512;
  }

  void DoorStorage::_serializeTo(const DoorSetting& model, DynamicJsonDocument& doc) {
    // open
    const JsonArray openArr = doc.createNestedArray(F("open"));
    openArr[0] = model.openSensorPin;
    openArr[1] = model.openTrueValue;
    // closed
    const JsonArray closedArr = doc.createNestedArray(F("closed"));
    closedArr[0] = model.closedSensorPin;
    closedArr[1] = model.closedTrueValue;
  }

  void DoorStorage::_deserializeFrom(DoorSetting& model, const DynamicJsonDocument& doc) {
    // open
    const auto openArr = doc[F("open")];
    model.openSensorPin = openArr[0];
    model.openTrueValue = openArr[1];
    // closed
    const auto closedArr = doc[F("closed")];
    model.closedSensorPin = closedArr[0];
    model.closedTrueValue = closedArr[1];
  }

} // namespace Victor::Components
