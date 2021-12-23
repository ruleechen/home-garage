#include <Arduino.h>
#include <RCSwitch.h>
#include <arduino_homekit_server.h>

#include <BuiltinLed.h>
#include <VictorOTA.h>
#include <VictorWifi.h>
#include <VictorRadio.h>
#include <VictorWeb.h>

#include "DoorStorage.h"
#include "DoorSenser.h"

using namespace Victor;
using namespace Victor::Components;

extern "C" homekit_characteristic_t chaName;
extern "C" homekit_characteristic_t targetDoorState;
extern "C" homekit_characteristic_t currentDoorState;
extern "C" homekit_server_config_t config;

BuiltinLed* builtinLed;
VictorRadio radioPortal;
VictorWeb webPortal(80);
RCSwitch mySwitch = RCSwitch();
DoorSenser* doorSenser;

void targetDoorStateSetter(const homekit_value_t value) {
	targetDoorState.value.int_value = value.int_value;
  auto state = DoorState(value.int_value);
	if (state == DoorStateOpen) {
    radioPortal.emit("open");
    builtinLed->turnOn();
	} else {
    radioPortal.emit("close");
		builtinLed->turnOff();
	}
  console.log().bracket("TargetState").section(String(state));
}

void setCurrentDoorState(DoorState state) {
	currentDoorState.value.int_value = state;
	homekit_characteristic_notify(&currentDoorState, currentDoorState.value);
  if (state == DoorStateOpen || state == DoorStateClosed) {
    targetDoorState.value.int_value = state;
    homekit_characteristic_notify(&targetDoorState, targetDoorState.value);
    radioPortal.emit("stop");
  }
  console.log().bracket("CurrentState").section(String(state));
}

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error(F("fs mount failed"));
  }

  builtinLed = new BuiltinLed();
  builtinLed->turnOn();

  victorOTA.setup();
  victorWifi.setup();

  // setup radio
  auto radioJson = radioStorage.load();
  if (radioJson.inputPin > 0) {
	  mySwitch.enableReceive(radioJson.inputPin);
  }
  if (radioJson.outputPin > 0) {
    mySwitch.enableTransmit(radioJson.outputPin);
  }
  radioPortal.onEmit = [](const RadioEmit& emit) {
    auto value = emit.value.toInt();
    mySwitch.send(value, 24);
    builtinLed->flash();
    console.log().bracket(F("radio"))
      .section(F("sent")).section(emit.value)
      .section(F("via channel")).section(String(emit.channel));
  };

  // setup web
  webPortal.onRequestStart = []() { builtinLed->turnOn(); };
  webPortal.onRequestEnd = []() { builtinLed->turnOff(); };
  webPortal.onRadioEmit = [](int index) { radioPortal.emit(index); };
  webPortal.setup();

  // setup homekit server
  auto hostName = victorWifi.getHostName();
  chaName.value.string_value = const_cast<char*>(hostName.c_str());
  targetDoorState.setter = targetDoorStateSetter;
  arduino_homekit_setup(&config);

  // setup door sensor
  auto doorJson = doorStorage.load();
  doorSenser = new DoorSenser(doorJson);
  doorSenser->onStateChange = setCurrentDoorState;
  setCurrentDoorState(doorSenser->readState());

  // done
  builtinLed->flash();
  console.log(F("setup complete"));
}

void loop(void) {
  webPortal.loop();
  doorSenser->loop();
  arduino_homekit_loop();
  if (mySwitch.available()) {
    auto value = String(mySwitch.getReceivedValue());
    auto channel = mySwitch.getReceivedProtocol();
    radioPortal.receive(value, channel);
    builtinLed->flash();
    console.log().bracket(F("radio"))
      .section(F("received")).section(value)
      .section(F("from channel")).section(String(channel));
    mySwitch.resetAvailable();
  }
}
