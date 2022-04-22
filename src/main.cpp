#include <Arduino.h>
#include <RCSwitch.h>
#include <arduino_homekit_server.h>

#include <GlobalHelpers.h>
#include <Console.h>
#include <BuiltinLed.h>
#include <VictorOTA.h>
#include <VictorWifi.h>
#include <VictorRadio.h>
#include <VictorWeb.h>

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

VictorRadio radioPortal;
VictorWeb webPortal(80);
RCSwitch mySwitch = RCSwitch();
DoorSensor* doorSensor;
String hostName;
String serialNumber;

String toDoorStateName(uint8_t state) {
  return (
    state == DoorStateOpen ?    F("Open") :
    state == DoorStateClosed ?  F("Closed") :
    state == DoorStateOpening ? F("Opening") :
    state == DoorStateClosing ? F("Closing") :
    state == DoorStateStopped ? F("Stopped") : F("Unknown")
  );
}

void setTargetDoorState(DoorState state) {
  ESP.wdtFeed();
  targetDoorState.value.uint8_value = state;
  homekit_characteristic_notify(&targetDoorState, targetDoorState.value);
  if (state == DoorStateOpen) {
    builtinLed.turnOn();
    if (currentDoorState.value.uint8_value != DoorStateOpen) {
      radioPortal.emit(F("open"));
    }
  } else if (state == DoorStateClosed) {
    builtinLed.turnOff();
    if (currentDoorState.value.uint8_value != DoorStateClosed) {
      radioPortal.emit(F("close"));
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
  targetDoorState.value.uint8_value = (state == DoorStateOpen || state == DoorStateOpening) ? DoorStateOpen : DoorStateClosed;
  if (notify) {
    homekit_characteristic_notify(&targetDoorState, targetDoorState.value);
    homekit_characteristic_notify(&currentDoorState, currentDoorState.value);
    if (state == DoorStateOpen || state == DoorStateClosed) {
      radioPortal.emit(F("stop"));
    }
  }
  console.log()
    .bracket(F("door"))
    .section(F("current"), toDoorStateName(state));
}

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error()
      .bracket(F("fs"))
      .section(F("mount failed"));
  }

  builtinLed.setup();
  builtinLed.turnOn();

  // setup radio
  const auto radioJson = radioStorage.load();
  if (radioJson.inputPin > 0) {
    mySwitch.enableReceive(radioJson.inputPin);
  }
  if (radioJson.outputPin > 0) {
    mySwitch.enableTransmit(radioJson.outputPin);
  }
  radioPortal.onEmit = [](const RadioEmit& emit) {
    const auto value = emit.value.toInt();
    mySwitch.setProtocol(emit.channel);
    mySwitch.send(value, 24);
    builtinLed.flash();
    console.log()
      .bracket(F("radio"))
      .section(F("sent"), emit.value)
      .section(F("via channel"), String(emit.channel));
  };

  // setup web
  webPortal.onRequestStart = []() { builtinLed.toggle(); };
  webPortal.onRequestEnd = []() { builtinLed.toggle(); };
  webPortal.onRadioEmit = [](uint8_t index) { radioPortal.emit(index); };
  webPortal.onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    // states
    states.push_back({ .text = F("Service"),     .value = VICTOR_ACCESSORY_SERVICE_NAME });
    states.push_back({ .text = F("Target"),      .value = toDoorStateName(targetDoorState.value.uint8_value) });
    states.push_back({ .text = F("Current"),     .value = toDoorStateName(currentDoorState.value.uint8_value) });
    states.push_back({ .text = F("Obstruction"), .value = GlobalHelpers::toYesNoName(obstructionState.value.bool_value) });
    states.push_back({ .text = F("Paired"),      .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"),     .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("Unpair"), .value = F("Unpair") });
    if (targetDoorState.value.uint8_value == DoorStateOpen) {
      buttons.push_back({ .text = F("Close"), .value = F("Close") });
    } else {
      buttons.push_back({ .text = F("Open"), .value = F("Open") });
    }
  };
  webPortal.onServicePost = [](const String& value) {
    if (value == F("Unpair")) {
      homekit_server_reset();
    } else if (value == F("Close")) {
      setTargetDoorState(DoorStateClosed);
    } else if (value == F("Open")) {
      setTargetDoorState(DoorStateOpen);
    }
  };
  webPortal.setup();

  // setup sensor
  const auto storage = new DoorStorage("/door.json");
  doorSensor = new DoorSensor(storage->load());
  doorSensor->onStateChange = [](const DoorState state) { setCurrentDoorState(state, true); };
  setCurrentDoorState(doorSensor->readState(), false);

  // setup homekit server
  hostName = victorWifi.getHostName();
  serialNumber = String(VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER) + "/" + victorWifi.getHostId();
  accessoryName.value.string_value = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value =const_cast<char*>(serialNumber.c_str());
  targetDoorState.setter = targetDoorStateSetter;
  arduino_homekit_setup(&serverConfig);

  // setup wifi
  victorOTA.setup();
  victorWifi.setup();

  // done
  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  arduino_homekit_loop();
  webPortal.loop();
  doorSensor->loop();
  // loop radio
  if (mySwitch.available()) {
    const auto value = String(mySwitch.getReceivedValue());
    const auto channel = mySwitch.getReceivedProtocol();
    radioPortal.receive(value, channel);
    builtinLed.flash();
    console.log()
      .bracket(F("radio"))
      .section(F("received"), value)
      .section(F("from channel"), String(channel));
    mySwitch.resetAvailable();
  }
}
