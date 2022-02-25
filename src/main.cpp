#include <Arduino.h>
#include <RCSwitch.h>
#include <arduino_homekit_server.h>

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
extern "C" homekit_server_config_t serverConfig;

VictorRadio radioPortal;
VictorWeb webPortal(80);
RCSwitch mySwitch = RCSwitch();
DoorSensor* doorSensor;
String hostName;

String toYesNoName(bool state) {
  return state == true ? F("Yes") : F("No");
}

String toDoorStateName(uint8_t state) {
  return (
    state == DoorStateOpen ?    F("Open") :
    state == DoorStateClosed ?  F("Closed") :
    state == DoorStateOpening ? F("Opening") :
    state == DoorStateClosing ? F("Closing") :
    state == DoorStateStopped ? F("Stopped") : F("Unknown")
  );
}

void targetDoorStateSetter(const homekit_value_t value) {
  ESP.wdtFeed();
  targetDoorState.value.uint8_value = value.uint8_value;
  homekit_characteristic_notify(&targetDoorState, targetDoorState.value);
  const auto state = DoorState(value.uint8_value);
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
  webPortal.onServiceGet = [](std::vector<KeyValueModel>& items) {
    items.push_back({ .key = F("Service"),     .value = VICTOR_ACCESSORY_SERVICE_NAME });
    items.push_back({ .key = F("Target"),      .value = toDoorStateName(targetDoorState.value.uint8_value) });
    items.push_back({ .key = F("Current"),     .value = toDoorStateName(currentDoorState.value.uint8_value) });
    items.push_back({ .key = F("Obstruction"), .value = toYesNoName(obstructionState.value.bool_value) });
    items.push_back({ .key = F("Paired"),      .value = toYesNoName(homekit_is_paired()) });
    items.push_back({ .key = F("Clients"),     .value = String(arduino_homekit_connected_clients_count()) });
  };
  webPortal.onServicePost = [](const String& value) {
    if (value == F("reset")) {
      homekit_server_reset();
    }
  };
  webPortal.setup();

  // setup sensor
  const auto doorJson = doorStorage.load();
  doorSensor = new DoorSensor(doorJson);
  doorSensor->onStateChange = [](const DoorState state) { setCurrentDoorState(state, true); };
  setCurrentDoorState(doorSensor->readState(), false);

  // setup homekit server
  hostName = victorWifi.getHostName();
  accessoryName.value.string_value = const_cast<char*>(hostName.c_str());
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
