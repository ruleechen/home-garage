#include "DoorStorage.h"

namespace Victor::Components {

  DoorStorage::DoorStorage() {
    _filePath = F("/door.json");
    _maxSize = 512;
  }

  void DoorStorage::_serializeTo(const DoorSetting& model, DynamicJsonDocument& doc) {
    const JsonArray item = doc.createNestedArray(F("s"));
    item[0] = model.openSenserPin;
    item[1] = model.closedSenserPin;
    item[2] = model.openTrueValue;
    item[3] = model.closedTrueValue;
  }

  void DoorStorage::_deserializeFrom(DoorSetting& model, const DynamicJsonDocument& doc) {
    const auto item = doc[F("s")];
    model.openSenserPin = item[0];
    model.closedSenserPin = item[1];
    model.openTrueValue = item[2];
    model.closedTrueValue = item[3];
  }

  // global
  DoorStorage doorStorage;

} // namespace Victor::Components
