#ifndef CONFIG_H
#define CONFIG_H

struct configData
{
	char projectName[32];
	char projectVersion[32];
	bool ShowFilePage;
	bool ShowFileFirmwarePage;
	bool ShowWebFirmwarePage;
	bool ShowDiagnosePage;
	char language[3];
	uint16_t MaxOnTime;
	uint16_t WarnOnTime;
	uint16_t PumpeStatusEinLevel;
	bool ZeitCheckInit;
	uint16_t FirebaseUpdateIntervall;
	char PrivateKeyFirst[4];
	bool ConnectToCloud;
	bool UseStream;
	uint16_t MaxAltdatenTage;
	char FirmwareURL[60];
	uint16_t Messagelevel;
	bool TemperaturEnable;
};

extern uint32_t configVersion;
extern const configData defaults;

#endif