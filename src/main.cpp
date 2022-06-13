#include <Arduino.h>
#include <RCSwitch.h>
#include <arduino_homekit_server.h>

#include <AppMain/AppMain.h>

#include "DoorStorage.h"
#include "DoorSensor.h"

using namespace Victor;
using namespace Victor::Components;

extern "C" homekit_characteristic_t targetDoorState;
extern "C" homekit_characteristic_t currentDoorState;
extern "C" homekit_characteristic_t obstructionState;
extern "C" homekit_characteristic_t accessoryName;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;

AppMain* appMain;

RCSwitch rf = RCSwitch();
DoorSensor* doorSensor;
uint16_t doorDebounce;
String hostName;
String serialNumber;

String toDoorStateName(uint8_t state) {
  return (
    state == DOOR_STATE_OPEN ?    F("Open") :
    state == DOOR_STATE_CLOSED ?  F("Closed") :
    state == DOOR_STATE_OPENING ? F("Opening") :
    state == DOOR_STATE_CLOSING ? F("Closing") :
    state == DOOR_STATE_STOPPED ? F("Stopped") : F("Unknown")
  );
}

void setTargetDoorState(DoorState state) {
  ESP.wdtFeed();
  targetDoorState.value.uint8_value = state;
  homekit_characteristic_notify(&targetDoorState, targetDoorState.value);
  if (state == DOOR_STATE_OPEN) {
    builtinLed.turnOn();
    if (currentDoorState.value.uint8_value != DOOR_STATE_OPEN) {
      appMain->radioPortal->emit(F("open"));
    }
  } else if (state == DOOR_STATE_CLOSED) {
    builtinLed.turnOff();
    if (currentDoorState.value.uint8_value != DOOR_STATE_CLOSED) {
      appMain->radioPortal->emit(F("close"));
    }
  }
  console.log()
    .bracket(F("door"))
    .section(F("target"), toDoorStateName(state));
}

void targetDoorStateSetter(const homekit_value_t value) {
  setTargetDoorState(DoorState(value.uint8_value));
}

void setCurrentDoorState(DoorState state, bool notify) {
  ESP.wdtFeed();
  currentDoorState.value.uint8_value = state;
  targetDoorState.value.uint8_value = (state == DOOR_STATE_OPEN || state == DOOR_STATE_OPENING) ? DOOR_STATE_OPEN : DOOR_STATE_CLOSED;
  if (notify) {
    homekit_characteristic_notify(&targetDoorState, targetDoorState.value);
    homekit_characteristic_notify(&currentDoorState, currentDoorState.value);
    if (state == DOOR_STATE_OPEN || state == DOOR_STATE_CLOSED) {
      delay(doorDebounce); // pause some time before emit stop command to wait for door really stopped
      appMain->radioPortal->emit(F("stop"));
    }
  }
  console.log()
    .bracket(F("door"))
    .section(F("current"), toDoorStateName(state));
}

void setup(void) {
  appMain = new AppMain();
  appMain->setup();

  // setup radio
  const auto radioJson = radioStorage.load();
  if (radioJson.inputPin > 0) {
    rf.enableReceive(radioJson.inputPin);
  }
  if (radioJson.outputPin > 0) {
    rf.enableTransmit(radioJson.outputPin);
  }
  appMain->radioPortal->onEmit = [](const RadioEmit& emit) {
    const auto value = emit.value.toInt();
    rf.setProtocol(emit.channel);
    rf.send(value, 24);
    builtinLed.flash();
    console.log()
      .bracket(F("radio"))
      .section(F("sent"), emit.value)
      .section(F("via channel"), String(emit.channel));
  };

  // setup web
  appMain->webPortal->onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    // states
    states.push_back({ .text = F("Service"),     .value = VICTOR_ACCESSORY_SERVICE_NAME });
    states.push_back({ .text = F("Target"),      .value = toDoorStateName(targetDoorState.value.uint8_value) });
    states.push_back({ .text = F("Current"),     .value = toDoorStateName(currentDoorState.value.uint8_value) });
    states.push_back({ .text = F("Obstruction"), .value = GlobalHelpers::toYesNoName(obstructionState.value.bool_value) });
    states.push_back({ .text = F("Paired"),      .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"),     .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("UnPair"),     .value = F("UnPair") });
    buttons.push_back({ .text = F("Door-Close"), .value = F("Close") });
    buttons.push_back({ .text = F("Door-Open"),  .value = F("Open") });
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    if (value == F("UnPair")) {
      homekit_server_reset();
      ESP.restart();
    } else if (value == F("Close")) {
      setTargetDoorState(DOOR_STATE_CLOSED);
    } else if (value == F("Open")) {
      setTargetDoorState(DOOR_STATE_OPEN);
    }
  };

  // setup sensor
  const auto storage = new DoorStorage("/door.json");
  const auto setting = storage->load();
  doorDebounce = setting.debounce;
  doorSensor = new DoorSensor(setting);
  doorSensor->onStateChange = [](const DoorState state) { setCurrentDoorState(state, true); };
  setCurrentDoorState(doorSensor->readState(), false);

  // setup homekit server
  hostName = victorWifi.getHostName();
  serialNumber = String(VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER) + "/" + victorWifi.getHostId();
  accessoryName.value.string_value = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  targetDoorState.setter = targetDoorStateSetter;
  arduino_homekit_setup(&serverConfig);

  // done
  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  appMain->loop();
  doorSensor->loop();
  arduino_homekit_loop();
  // loop radio
  if (rf.available()) {
    const auto value = String(rf.getReceivedValue());
    const auto channel = rf.getReceivedProtocol();
    appMain->radioPortal->receive(value, channel);
    builtinLed.flash();
    console.log()
      .bracket(F("radio"))
      .section(F("received"), value)
      .section(F("from channel"), String(channel));
    rf.resetAvailable();
  }
}
