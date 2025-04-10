#ifndef DASH_H
#define DASH_H

struct dashboardData
{
	bool ModulQuery;
	bool SendSMS;
	bool CheckSMS;
	bool DeleteSMS;
	char Operator[20];
	char SignalStrength[30];
	float BatteryVoltage;
	float Capacity;
	bool SleepMode;
	bool ReInit;
	bool GSMModulPower;
	bool Testschalter;
	bool StartMotionDetection;
	bool GotoDeepSleep;
	bool QueryTest;
	bool MessageTest;
	bool AltdatenLoeschen;
	uint16_t AnzeigeinMin;
};

#endif