/*
 * simple_led.ino
 *
 * This accessory contains a builtin-led on NodeMCU and a "virtual" occupancy sensor.
 * Setup code: 111-11-111
 * The Flash-Button(D3, GPIO0) on NodeMCU:
 * 		single-click: turn on/off the builtin-led (D4, GPIO2)
 * 		double-click: toggle the occupancy sensor state
 * 		long-click: reset the homekit server (remove the saved pairing)
 *
 *  Created on: 2020-02-08
 *      Author: Mixiaoxiao (Wang Bin)
 */

#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>

#include <arduino_homekit_server.h>

#include "wifi.h"

//D0 16 //led
//D3  0 //flash button
//D4  2 //led

#define PIN_LED 16//D0

#define LEDS 120

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

#define SIMPLE_INFO(fmt, ...)   printf(fmt "\n" , ##__VA_ARGS__);

extern "C" void led_update(void);

int leds_bri = 100; //[0, 100]
bool leds_power = false; //true or false
float leds_hue = 0.0f; //[0, 360]
float leds_sat = 100.0f; //[0, 100]

#include "hsb_to_rgb.c"

unsigned char setBrightness(unsigned char level) {
	// set levels from 0 to 31
	return (0xe0 | level);
}

unsigned char getBrightness(unsigned char level) {
	return (0x1f & level);
}

void spi_transfer() {
	int colors[3];
	hsb2rgb(leds_hue, leds_sat, leds_bri, colors);
	SIMPLE_INFO("Update: (%.2f, %.2f, %d), %02x%02x%02x", leds_hue, leds_sat, leds_bri, colors[0], colors[1], colors[2]);
	for (int i = 0; i <= 3; i++) {
		SPI.transfer(0x00);
	}
	for (int i = 0; i < LEDS; i++) {
		SPI.transfer(31);
		SPI.transfer(colors[2]);
		SPI.transfer(colors[1]);
		SPI.transfer(colors[0]);
	}
	for (int i = 0; i < LEDS/2; i++) {
		SPI.transfer(0x01);
	}
}

extern "C" void leds_update(void) {
	if (leds_power) {
		printf("ON %d\n", leds_bri);
	} else {
		printf("OFF\n");
		leds_bri = 0;
	}
	spi_transfer();
}

void blink_led(int interval, int count) {
	for (int i = 0; i < count; i++) {
		builtinledSetStatus(true);
		delay(interval);
		builtinledSetStatus(false);
		delay(interval);
	}
}

void setup() {
	Serial.begin(115200);
	Serial.setRxBufferSize(32);
	Serial.setDebugOutput(false);
	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	pinMode(PIN_LED, OUTPUT);
	WiFi.mode(WIFI_STA);
	WiFi.persistent(false);
	WiFi.disconnect(false);
	WiFi.setAutoReconnect(true);
	WiFi.begin(ssid, password);

	SIMPLE_INFO("");
	SIMPLE_INFO("SketchSize: %d", ESP.getSketchSize());
	SIMPLE_INFO("FreeSketchSpace: %d", ESP.getFreeSketchSpace());
	SIMPLE_INFO("FlashChipSize: %d", ESP.getFlashChipSize());
	SIMPLE_INFO("FlashChipRealSize: %d", ESP.getFlashChipRealSize());
	SIMPLE_INFO("FlashChipSpeed: %d", ESP.getFlashChipSpeed());
	SIMPLE_INFO("SdkVersion: %s", ESP.getSdkVersion());
	SIMPLE_INFO("FullVersion: %s", ESP.getFullVersion().c_str());
	SIMPLE_INFO("CpuFreq: %dMHz", ESP.getCpuFreqMHz());
	SIMPLE_INFO("FreeHeap: %d", ESP.getFreeHeap());
	SIMPLE_INFO("ResetInfo: %s", ESP.getResetInfo().c_str());
	SIMPLE_INFO("ResetReason: %s", ESP.getResetReason().c_str());
	INFO_HEAP();
	homekit_setup();
	INFO_HEAP();
	blink_led(200, 3);
}

void loop() {
	homekit_loop();
}

void builtinledSetStatus(bool on) {
	digitalWrite(PIN_LED, on ? LOW : HIGH);
}

//==============================
// Homekit setup and loop
//==============================

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t name;
extern "C" void accessory_init();

void homekit_setup() {
	accessory_init();
	uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(mac);
	int name_len = snprintf(NULL, 0, "%s_%02X%02X%02X",
			name.value.string_value, mac[3], mac[4], mac[5]);
	char *name_value = (char*) malloc(name_len + 1);
	snprintf(name_value, name_len + 1, "%s_%02X%02X%02X",
			name.value.string_value, mac[3], mac[4], mac[5]);
	name.value = HOMEKIT_STRING_CPP(name_value);

	arduino_homekit_setup(&config);
	SIMPLE_INFO("HomeKit Setup done");
}

void homekit_loop() {
	arduino_homekit_loop();
	static uint32_t next_heap_millis = 0;
	uint32_t time = millis();
	if (time > next_heap_millis) {
		SIMPLE_INFO("heap: %d, sockets: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
		next_heap_millis = time + 5000;
	}
}
