#include <Arduino.h>
#include "config.h"

uint32_t configVersion = 3254419057; //generated identifier to compare config with EEPROM

const configData defaults PROGMEM =
{
	"GSM/GPS-Tracker",
	"V0.02",
	false,
	true,
	false,
	true,
	"de",
	300,
	120,
	200,
	true,
	15000,
	"",
	false,
	false,
	330,
	"http://www.bierhoehle.de/gsmtracker.bin",
	2
};