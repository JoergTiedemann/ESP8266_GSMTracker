#include <Arduino.h>
#include "config.h"

uint32_t configVersion = 1409788380; //generated identifier to compare config with EEPROM

const configData defaults PROGMEM =
{
	"GSM/GP-Tracker",
	"V0.01",
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
	"http://www.bierhoehle.de/pumpe.bin",
	2,
	true
};