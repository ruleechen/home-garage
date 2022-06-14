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
bool connective;

RCSwitch rf = RCSwitch();
DoorSensor* doorSensor;
uint16_t doorAutoStop;

String hostName;
String serialNumber;

String toDoorStateName(const uint8_t state) {
  return (
    state == 0 ? F("Open") :
    state == 1 ? F("Closed") :
    state == 2 ? F("Opening") :
    state == 3 ? F("Closing") :
    state == 4 ? F("Stopped") : F("Unknown")
  );
}

void emitDoorCommand(const DoorCommand command) {
  switch (command) {
    case DOOR_COMMAND_OPEN:
      appMain->radioPortal->emit(F("open"));
      break;
    case DOOR_COMMAND_CLOSE:
      appMain->radioPortal->emit(F("close"));
      break;
    case DOOR_COMMAND_STOP:
      appMain->radioPortal->emit(F("stop"));
      break;
    default:
      break;
  }
}

void setTargetDoorState(const TargetDoorState targetState, const bool notify) {
  ESP.wdtFeed();
  targetDoorState.value.uint8_value = targetState;
  if (notify) {
    homekit_characteristic_notify(&targetDoorState, targetDoorState.value);
  }
  if (targetState == TARGET_DOOR_STATE_OPEN) {
    builtinLed.turnOn(); // warning
    if (currentDoorState.value.uint8_value != CURRENT_DOOR_STATE_OPEN) {
      emitDoorCommand(DOOR_COMMAND_OPEN);
    }
  } else if (targetState == TARGET_DOOR_STATE_CLOSED) {
    builtinLed.turnOff(); // safe
    if (currentDoorState.value.uint8_value != CURRENT_DOOR_STATE_CLOSED) {
      emitDoorCommand(DOOR_COMMAND_CLOSE);
    }
  }
  console.log()
    .bracket(F("door"))
    .section(F("target"), toDoorStateName(targetState));
}

void setCurrentDoorState(const CurrentDoorState currentState, const bool notify) {
  ESP.wdtFeed();
  currentDoorState.value.uint8_value = currentState;
  if (currentState == CURRENT_DOOR_STATE_OPEN || currentState == CURRENT_DOOR_STATE_OPENING) {
    targetDoorState.value.uint8_value = TARGET_DOOR_STATE_OPEN;
  } else if (currentState == CURRENT_DOOR_STATE_CLOSED || currentState == CURRENT_DOOR_STATE_CLOSING) {
    targetDoorState.value.uint8_value = TARGET_DOOR_STATE_CLOSED;
  }
  if (notify) {
    homekit_characteristic_notify(&targetDoorState, targetDoorState.value);
    homekit_characteristic_notify(&currentDoorState, currentDoorState.value);
  }
  if (
    currentState == CURRENT_DOOR_STATE_OPEN ||
    currentState == CURRENT_DOOR_STATE_CLOSED
  ) {
    if (doorAutoStop > 0) {
      // pause some time before emit stop command to wait for door really stopped
      if (doorAutoStop > 1) { delay(doorAutoStop); }
      // emit stop command
      emitDoorCommand(DOOR_COMMAND_STOP);
    }
  } else if (currentState == CURRENT_DOOR_STATE_STOPPED) {
    // stop directly
    emitDoorCommand(DOOR_COMMAND_STOP);
  }
  console.log()
    .bracket(F("door"))
    .section(F("current"), toDoorStateName(currentState));
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
    buttons.push_back({ .text = F("Door-Open"),  .value = F("Open") });
    buttons.push_back({ .text = F("Door-Close"), .value = F("Close") });
    buttons.push_back({ .text = F("Door-Stop"),  .value = F("Stop") });
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    if (value == F("UnPair")) {
      homekit_server_reset();
      ESP.restart();
    } else if (value == F("Open")) {
      setTargetDoorState(TARGET_DOOR_STATE_OPEN, connective);
    } else if (value == F("Close")) {
      setTargetDoorState(TARGET_DOOR_STATE_CLOSED, connective);
    } else if (value == F("Stop")) {
      setCurrentDoorState(CURRENT_DOOR_STATE_STOPPED, connective);
    }
  };

  // setup sensor
  const auto storage = new DoorStorage("/door.json");
  const auto setting = storage->load();
  doorAutoStop = setting.autoStop;
  doorSensor = new DoorSensor(setting);
  doorSensor->onStateChange = [](const CurrentDoorState currentState) { setCurrentDoorState(currentState, connective); };
  setCurrentDoorState(doorSensor->readState(), false);

  // setup homekit server
  hostName = victorWifi.getHostName();
  serialNumber = String(VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER) + "/" + victorWifi.getHostId();
  accessoryName.value.string_value = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  targetDoorState.setter = [](const homekit_value_t value) { setTargetDoorState(TargetDoorState(value.uint8_value), connective); };
  arduino_homekit_setup(&serverConfig);

  // done
  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  arduino_homekit_loop();
  const auto isPaired = arduino_homekit_get_running_server()->paired;
  connective = victorWifi.isLightSleepMode() && isPaired;
  appMain->loop(connective);
  doorSensor->loop();
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
