/*
 * simple_leds_accessory.c
 * Define the accessory in pure C language using the Macro in characteristics.h
 *
 *  Created on: 2020-02-08
 *      Author: Mixiaoxiao (Wang Bin)
 *  Edited on: 2020-03-01
 *      Edited by: euler271 (Jonas Linn)
 */

#include <Arduino.h>
#include <homekit/types.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <stdio.h>
#include <port.h>

//const char * buildTime = __DATE__ " " __TIME__ " GMT";

#include "homekit_config.h"

extern int leds_bri; //[0, 100]
extern bool leds_power; //true or false
extern float leds_hue; //[0, 360]
extern float leds_sat; //[0, 100]

extern void led_update(void);

// Characteristic ON
homekit_value_t leds_on_get() {
	return HOMEKIT_BOOL(leds_power);
}

void leds_on_set(homekit_value_t value) {
	if (value.format != homekit_format_bool) {
		printf("Invalid on-value format: %d\n", value.format);
		return;
	}
	leds_on_set_bool(value.bool_value);
}

void leds_on_set_bool(bool state) {
	if (state && leds_bri == 0) {
		leds_bri = 100;
	}
	leds_power = state;
	leds_update();
}

// Characteristic BRIGHTNESS
homekit_value_t leds_bri_get() {
	return HOMEKIT_INT(leds_bri);
}

void leds_bri_set(homekit_value_t value) {
	if (value.format != homekit_format_int) {
		printf("Invalid bri-value format: %d\n", value.format);
		return;
	}
	printf("set bri: %d", value.int_value);
	leds_bri = value.int_value;
	leds_on_set_bool(leds_bri > 0);
}

// Characteristic HUE
homekit_value_t leds_hue_get() {
	printf("get hue: %.2f", leds_hue);
	return HOMEKIT_FLOAT(leds_hue);
}

void leds_hue_set(homekit_value_t value) {
	if (value.format != homekit_format_float) {
		printf("Invalid hue-value format: %d\n", value.format);
		return;
	}
	printf("set hue: %.2f", value.float_value);
	leds_hue = value.float_value;
	leds_on_set_bool(true);
}

// Characteristic SATURATION
homekit_value_t leds_sat_get() {
	printf("get sat: %.2f", leds_sat);
	return HOMEKIT_FLOAT(leds_sat);
}

void leds_sat_set(homekit_value_t value) {
	if (value.format != homekit_format_float) {
		printf("Invalid sat-value format: %d\n", value.format);
		return;
	}
	printf("set sat: %.2f", value.float_value);
	leds_sat = value.float_value;
	leds_on_set_bool(true);
}

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, ACCESSORY_NAME);
homekit_characteristic_t serial_number = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, ACCESSORY_SN);

void accessory_identify(homekit_value_t _value) {
	printf("accessory identify\n");
	for (int j = 0; j < 3; j++) {
		leds_power = true;
		leds_update();
		delay(100);
		leds_power = false;
		leds_update();
		delay(100);
	}
}

homekit_accessory_t *accessories[] =
		{
				HOMEKIT_ACCESSORY(
						.id = 1,
						.category = homekit_accessory_category_lightbulb,
						.services=(homekit_service_t*[]){
						  HOMEKIT_SERVICE(ACCESSORY_INFORMATION,
						  .characteristics=(homekit_characteristic_t*[]){
						    &name,
						    HOMEKIT_CHARACTERISTIC(MANUFACTURER, ACCESSORY_MANUFACTURER),
						    &serial_number,
						    HOMEKIT_CHARACTERISTIC(MODEL, ACCESSORY_MODEL),
						    HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.0.1"),
						    HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
						    NULL
						  }),
						  HOMEKIT_SERVICE(LIGHTBULB, .primary=true,
						  .characteristics=(homekit_characteristic_t*[]){
						    HOMEKIT_CHARACTERISTIC(NAME, "Led"),
						    HOMEKIT_CHARACTERISTIC(ON, false, .getter=leds_on_get, .setter=leds_on_set),
						    HOMEKIT_CHARACTERISTIC(HUE, 0.0f, .getter=leds_hue_get, .setter=leds_hue_set),
						    HOMEKIT_CHARACTERISTIC(SATURATION, 0.0f, .getter=leds_sat_get, .setter=leds_sat_set),
						    HOMEKIT_CHARACTERISTIC(BRIGHTNESS, 0, .getter=leds_bri_get, .setter=leds_bri_set),
						    NULL
						  }),
						  NULL
						}),
				NULL
		};

homekit_server_config_t config = {
		.accessories = accessories,
		.password = ACCESSORY_PASSWORD,
		//.on_event = on_homekit_event,
		.setupId = "ABCD"
};

void accessory_init() {
	leds_update();
}
