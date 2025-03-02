#ifndef DASH_H
#define DASH_H

struct dashboardData
{
	bool ModulQuery;
	bool SendSMS;
	bool CheckSMS;
	uint16_t aktuelleLaufzeit;
	float Temperatur;
	bool Pumpenzustand;
	bool PumpenAbschaltError;
	bool RuntimeMonitor;
	bool Testschalter;
	bool QueryTest;
	bool MessageTest;
	bool AltdatenLoeschen;
	uint16_t AnzeigeinMin;
};

#endif