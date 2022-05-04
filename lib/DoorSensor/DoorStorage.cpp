#include "DoorStorage.h"

namespace Victor::Components {

  DoorStorage::DoorStorage(const char* filePath) {
    _filePath = filePath;
    _maxSize = 512;
  }

  void DoorStorage::_serializeTo(const DoorSetting& model, DynamicJsonDocument& doc) {
    // door open
    const JsonArray openArr = doc.createNestedArray(F("open"));
    openArr[0] = model.doorOpenPin;
    openArr[1] = model.doorOpenTrueValue;
    // door closed
    const JsonArray closedArr = doc.createNestedArray(F("closed"));
    closedArr[0] = model.doorClosedPin;
    closedArr[1] = model.doorClosedTrueValue;
  }

  void DoorStorage::_deserializeFrom(DoorSetting& model, const DynamicJsonDocument& doc) {
    // door open
    const auto openArr = doc[F("open")];
    model.doorOpenPin = openArr[0];
    model.doorOpenTrueValue = openArr[1];
    // door closed
    const auto closedArr = doc[F("closed")];
    model.doorClosedPin = closedArr[0];
    model.doorClosedTrueValue = closedArr[1];
  }

} // namespace Victor::Components
