#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void my_accessory_identify(homekit_value_t _value) {
	printf("accessory identify\n");
}

homekit_characteristic_t targetDoorState = HOMEKIT_CHARACTERISTIC_(TARGET_DOOR_STATE, 0);
homekit_characteristic_t currentDoorState = HOMEKIT_CHARACTERISTIC_(CURRENT_DOOR_STATE, 0);
// homekit_characteristic_t cha_obstruction_detection = HOMEKIT_CHARACTERISTIC_(OBSTRUCTION_DETECTION, false);
homekit_characteristic_t chaName = HOMEKIT_CHARACTERISTIC_(NAME, VICTOR_HOMEKIT_CHARACTERISTIC_NAME);

homekit_accessory_t *accessories[] = {
  HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_garage, .services=(homekit_service_t*[]) {
    HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, VICTOR_HOMEKIT_CHARACTERISTIC_NAME),
      HOMEKIT_CHARACTERISTIC(MANUFACTURER, VICTOR_ACCESSORY_INFORMATION_MANUFACTURER),
      HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER),
      HOMEKIT_CHARACTERISTIC(MODEL, VICTOR_ACCESSORY_INFORMATION_MODEL),
      HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, VICTOR_FIRMWARE_VERSION),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
      NULL
    }),
    HOMEKIT_SERVICE(GARAGE_DOOR_OPENER, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
      &targetDoorState,
      &currentDoorState,
      // &cha_obstruction_detection,
      &chaName,
      NULL
    }),
    NULL
  }),
  NULL
};

homekit_server_config_t config = {
  .accessories = accessories,
  .password = VICTOR_HOMEKIT_SERVER_PASSWORD
};
