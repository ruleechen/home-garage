#include <Arduino.h>
#include <RCSwitch.h>

#include "BuiltinLed.h"
#include "VictorOTA.h"
#include "VictorWifi.h"
#include "VictorRadio.h"
#include "VictorWeb.h"

using namespace Victor;
using namespace Victor::Components;

BuiltinLed* builtinLed;
VictorRadio radioPortal;
VictorWeb webPortal(80);
RCSwitch mySwitch = RCSwitch();

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error(F("fs mount failed"));
  }

  builtinLed = new BuiltinLed();
  builtinLed->turnOn();

  victorOTA.setup();
  victorWifi.setup();

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

  webPortal.onRequestStart = []() { builtinLed->turnOn(); };
  webPortal.onRequestEnd = []() { builtinLed->turnOff(); };
  webPortal.onRadioEmit = [](int index) { radioPortal.emit(index); };
  webPortal.setup();

  builtinLed->flash();
  console.log(F("setup complete"));
}

void loop(void) {
  webPortal.loop();
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
