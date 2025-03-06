#include <Arduino.h>
#include "config.h"

uint32_t configVersion = 2539015658; //generated identifier to compare config with EEPROM

const configData defaults PROGMEM =
{
	"GSM/GPS-Tracker",
	"V1.00",
	false,
	true,
	false,
	true,
	"de",
	"01747318866",
	"Das ist eine Testsms vom SIM800L",
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