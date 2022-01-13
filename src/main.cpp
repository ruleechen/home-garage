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
#include "DoorSenser.h"

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
DoorSenser* doorSenser;
String hostName;

String parseStateName(int state) {
  return state == DoorStateOpen ? F("Open")
    : state == DoorStateClosed ?  F("Closed")
    : state == DoorStateOpening ? F("Opening")
    : state == DoorStateClosing ? F("Closing")
    : state == DoorStateStopped ? F("Stopped")
    : F("Unknown");
}

String parseYesNo(bool state) {
  return state == true ? F("Yes") : F("No");
}

void targetDoorStateSetter(const homekit_value_t value) {
  targetDoorState.value.int_value = value.int_value;
  const auto state = DoorState(value.int_value);
  if (state == DoorStateOpen) {
    builtinLed.turnOn();
    if (currentDoorState.value.int_value != DoorStateOpen) {
      radioPortal.emit(F("open"));
    }
  } else if (state == DoorStateClosed) {
    builtinLed.turnOff();
    if (currentDoorState.value.int_value != DoorStateClosed) {
      radioPortal.emit(F("close"));
    }
  }
  console.log()
    .bracket(F("door"))
    .section(F("target"), parseStateName(state));
}

void setCurrentDoorState(DoorState state, bool notify) {
  currentDoorState.value.int_value = state;
  targetDoorState.value.int_value = (state == DoorStateOpen || state == DoorStateOpening) ? DoorStateOpen : DoorStateClosed;
  if (notify) {
    homekit_characteristic_notify(&targetDoorState, targetDoorState.value);
    homekit_characteristic_notify(&currentDoorState, currentDoorState.value);
    if (state == DoorStateOpen || state == DoorStateClosed) {
      radioPortal.emit(F("stop"));
    }
  }
  console.log()
    .bracket(F("door"))
    .section(F("current"), parseStateName(state));
}

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error(F("fs mount failed"));
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
  webPortal.onRadioEmit = [](const int index) { radioPortal.emit(index); };
  webPortal.onServiceGet = [](std::vector<KeyValueModel>& items) {
    items.push_back({ .key = F("Service"), .value = VICTOR_ACCESSORY_SERVICE_NAME });
    items.push_back({ .key = F("State"),   .value = parseStateName(currentDoorState.value.int_value) });
    items.push_back({ .key = F("Paired"),  .value = parseYesNo(homekit_is_paired()) });
    items.push_back({ .key = F("Clients"), .value = String(arduino_homekit_connected_clients_count()) });
  };
  webPortal.onServicePost = [](const String type) {
    if (type == F("reset")) {
      homekit_server_reset();
    }
  };
  webPortal.setup();

  // setup door sensor
  const auto doorJson = doorStorage.load();
  doorSenser = new DoorSenser(doorJson);
  doorSenser->onStateChange = [](const DoorState state) { setCurrentDoorState(state, true); };
  setCurrentDoorState(doorSenser->readState(), false);

  // setup homekit server
  hostName = victorWifi.getHostName();
  accessoryName.value.string_value = const_cast<char*>(hostName.c_str());
  targetDoorState.setter = targetDoorStateSetter;
  arduino_homekit_setup(&serverConfig);

  // setup wifi
  victorOTA.setup();
  victorWifi.setup();

  // done
  console.log(F("setup complete"));
}

void loop(void) {
  webPortal.loop();
  doorSenser->loop();
  arduino_homekit_loop();
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
