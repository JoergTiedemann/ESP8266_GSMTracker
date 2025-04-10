#include <Arduino.h>

#include "FS.h"
#include <LittleFS.h>

// #ifdef ESP32
//     #include "LITTLEFS.h"   // https://github.com/lorol/LITTLEFS/tree/master/examples/LITTLEFS_PlatformIO
//     #define LittleFS LITTLEFS
// #elif defined(ESP8266)
//     #include "LittleFS.h"
//     #include <TZ.h>
// #endif

#include "WiFiManager.h"
#include "webServer.h"
#include "updater.h"
#include "configManager.h"
#include "timeSync.h"
#include <TZ.h>

#include "dashboard.h"
#include "DiagManager.h"
#include "BoardsInformation.h"
#include "GyroManager.h"

#include <Sim800L.h>
#include <SoftwareSerial.h>               

#define RX  14 //D5
#define TX  12 // D6
#define LED_PIN 2 // D4 ist GPIO 2
#define GSMPOWER_PIN 13 // D7 ist GPIO 13

Sim800L GSM(RX, TX);
CGyroManager GyroManager;

char buffer [80];

String strTime("No Time");
String strBootTime("");
String m_strOperator = "No Operator";
String m_strSignalQuality = "";
bool m_bGSMPowerState = false;

uint32_t m_FreeHeap =0;
bool b1 = false;
struct tm * timeinfo = NULL;
time_t now = 0;

struct task
{    
    unsigned long rate;
    unsigned long previous;
};

task taskA = { .rate = 500, .previous = 0 };
task taskCloud = { .rate = 1000, .previous = 0 };

void saveCallback() {
    Serial.println("EEPROM saved"); 
    DiagManager.PushDiagData(msgAll,"Konfiguration in EEPROM gesichert");
}


void GSMInit()
{
    // GSM.begin(9600);
    // GSM.setDebugLevel(cSeriellDebug);     
    delay(2000);    
    // Echo ausschalten
    if (configManager.data.SerialDebug > 0)
        GSM.setDebugLevel(cSeriellDebug);
    else
        GSM.setDebugLevel(cNoDebug);

    // wir schauen zunächst mal das das Modem auf AT Befehle reagiert
    String str = "";
    bool bReaktion = false;
    uint64_t startVersuchszeit = millis();
    while (!bReaktion)
    {
        GSM.printSerial("AT\r");
        uint64_t startTime = millis();
        while ((str.indexOf("OK")) == -1)
        {
            if (millis() - startTime > 500) {
             Serial.println("Timeout beim Warten auf OK");
             break; // Timeout erreicht, Schleife verlassen
            }
        str += GSM._readSerial(500);
        }
        if ((str.indexOf("OK")) != -1)
            bReaktion = true;
        if (millis() - startVersuchszeit > 10000)
        {
            Serial.println("GSM Modul reagiert nicht auf AT Befehle");
            return;
        }
    }
    Serial.printf("bReaktion:%d\n",bReaktion);

    Serial.println("Echo ausschalten: ATE0");
    GSM.sendATCommand("ATE0",10000);
    GSM.EnableEinbuchungsmessage(true);

    GSM.prepareForSmsReceive();

}

void setup() 
{
    Serial.begin(115200);
    BoardInformation.PrintBoardInformation();
    BoardInformation.print_used_libraries();

    String resetReason = ESP.getResetReason();
    Serial.println("Resetgrund: " + resetReason);
    Serial.println("===========================");
    struct rst_info *resetInfo = system_get_rst_info();
    Serial.print("Resetgrund: ");
    Serial.println(resetInfo->reason);
    Serial.print("Exccause: ");
    Serial.println(resetInfo->exccause);
    Serial.print("Epcr: ");
    Serial.println(resetInfo->epc1);
    // Interpretieren des exccause-Werts
    switch (resetInfo->exccause) {
        case 0:
            Serial.println("Illegal Instruction");
            break;
        case 1:
            Serial.println("Syscall");
            break;
        case 2:
            Serial.println("Instruction Fetch Error");
            break;
        case 3:
            Serial.println("Load Store Error");
            break;
        case 4:
            Serial.println("Level 1 Interrupt");
            break;
        case 5:
            Serial.println("Alloca");
            break;
        case 6:
            Serial.println("Integer Divide by Zero");
            break;
        case 7:
            Serial.println("PC Alignment");
            break;
        case 8:
            Serial.println("Data Alignment");
            break;
        default:
            Serial.println("Unknown Exception");
            break;
    }


    LittleFS.begin();
    configManager.begin();
    configManager.setConfigSaveCallback(saveCallback);
    WiFiManager.begin(configManager.data.projectName,240000);
    GUI.begin(BoardInformation.SendLibraryVersion);

    DiagManager.begin(20,10);

    Serial.println("Hello world, setup");

    if (!WiFiManager.isCaptivePortal())
    {
        // Configure device time using NTP server, with Amsterdam timezone
        #ifdef ESP32
            // ESP32 sntp APIs expect posix based time zone expressions, see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
            // Zeitzone Berlin
            timeSync.begin(PSTR("CET-1CEST,M3.5.0,M10.5.0/3"));
            //Wait for connection
            timeSync.waitForSyncResult(5000);
        #elif defined(ESP8266)
            timeSync.begin(TZ_Europe_Berlin);
        #endif

        #ifdef ESP32
            // use asctime to serialize local time to string
            now = time(nullptr);
            struct tm * timeinfo;
            timeinfo = localtime(&now);  
            char buffer [80];
            strftime (buffer,80,"%H:%M:%S %d.%m.%y:",timeinfo);
            strTime = String(buffer);
            Serial.print(PSTR("Current time in Berlin: "));
            Serial.println(strTime);
        #elif defined(ESP8266)
            //Wait for connection
            timeSync.waitForSyncResult(10000);

            if (timeSync.isSynced())
            {
                time_t now = time(nullptr);
                Serial.print(PSTR("Current time in Amsterdam: "));
                Serial.print(asctime(localtime(&now)));
            }
            else 
            {
                Serial.println("Timeout while receiving the time");
            }
        #endif
    }

    GyroManager.begin();


    dash.begin(500);
    // OTAManager.begin();

    DiagManager.AddVariableToMonitor(0,String("Letzter Systemstart"),&strBootTime);
    DiagManager.AddVariableToMonitor(2,String("Sytemzeit"),&strTime);
    DiagManager.AddVariableToMonitor(3,String("Free Heap"),&m_FreeHeap);
    // DiagManager.AddVariableToMonitor(1,String("Temperatur"),&m_Temperatur);
    // DiagManager.AddVariableToMonitor(5,String("akt. Tageslaufzeit"),&m_aktuelleTagesLaufzeit);
    // DiagManager.AddVariableToMonitor(6,String("PumpeIsRunning"),&m_PumpeIsRunning);
    // DiagManager.AddVariableToMonitor(8,String("ComboMode"),&dash.data.mode);
    // DiagManager.AddVariableToMonitor(9,String("CfgTestCombo"),&configManager.data.TestCombo);

    m_strOperator.toCharArray(dash.data.Operator,20);

    pinMode(LED_PIN, OUTPUT); // LED-Pin als Ausgang setzen
    digitalWrite(LED_PIN, HIGH); // LED ausschalten
    pinMode(GSMPOWER_PIN, OUTPUT); // LED-Pin als Ausgang setzen
    digitalWrite(GSMPOWER_PIN, HIGH); // GSM abschalten
    m_bGSMPowerState = false;
    dash.data.GSMModulPower = true;
    GSM.begin(9600);
    // GSMInit();
    GyroManager.InitWakeOnMotion();

    Serial.println("ready");

}

/*
wenn irgendwo localtTime verwendet wird, dann wird eine interne Variable der Lib besetzt und damit wird 
unser Intervallspeicher timeinfo verbogen, deshalb ziehen wir den hier wieder gerade und besetzen ihn mit 
der gueltigen Zeit
d.h. RestoreIntervallPointer soll überall nach localtime aufgerufen werden damit der IntervallPointer wieder richtig sitzt
*/
void RestoreIntervallPointer()
{
    now = time(nullptr);
    timeinfo = localtime(&now);  
} 

int i = 0;

void loop() 
{
    // Serial.print("Hello world, loop ");
    // Serial.println(i++);
    // delay(1000);
    m_FreeHeap = ESP.getFreeHeap();
    // if (timeSync.isSynced())

    struct tm timeinfo2;
    if(getLocalTime(&timeinfo2))
    {
        now = time(nullptr);
        //strTime = String(asctime(localtime(&now)));
        timeinfo = localtime(&now);  
        char buffer [80];
        //strftime (buffer,80,"%d.%m.%y %H:%M:%S",timeinfo);
        strftime (buffer,80,"%H:%M:%S",timeinfo);
        strTime = String(buffer);
        if (strBootTime == "")
        {
            strftime (buffer,80,"%d.%m.%y %H:%M:%S",timeinfo);
            strBootTime = String(buffer);
            Serial.println(String("BootTime:") += strBootTime);
        }
    }
    else 
    {
        strTime = String("No Time");
        DiagManager.PushDiagData(msgFehler,"No Time");
    }

    //software interrupts
    WiFiManager.loop();
    updater.loop();
    dash.loop();
    configManager.loop();
    GyroManager.loop();

    //your code here
    //task A
    if (taskA.previous == 0 || (millis() - taskA.previous > taskA.rate))
    {
        taskA.previous = millis();

        if (dash.data.ModulQuery)
        {
            // digitalWrite(LED_PIN, LOW); // LED einschalten
            dash.data.ModulQuery = false;
            m_strOperator = GSM.getOperator();
            m_strOperator.toCharArray(dash.data.Operator,20);
            Serial.println("GET OPERATOR: ");
            Serial.println(m_strOperator);
            m_strSignalQuality = GSM.signalQuality();
            m_strSignalQuality.toCharArray(dash.data.SignalStrength,30);
            // Serial.println("GET OPERATORS LIST: ");
            // Serial.println(GSM.getOperatorsList());
            Serial.println("GET PRODUCT INFO: ");
            Serial.println(GSM.getProductInfo());
            // digitalWrite(LED_PIN, HIGH); // LED ausschalten
        }

        if (dash.data.SendSMS)
        {
            // digitalWrite(LED_PIN, LOW); // LED einschalten
            Serial.printf("SMS Senden:%s\n",configManager.data.SMSText);
            dash.data.SendSMS = false;
            bool error = false; 					//to catch the response of sendSms
            error=GSM.sendSms(configManager.data.DialNumber,configManager.data.SMSText);
            Serial.printf("SendenSMS:%d\n",error);
            // digitalWrite(LED_PIN, HIGH); // LED ausschalten

        }

        if (dash.data.DeleteSMS)
        {
            // digitalWrite(LED_PIN, LOW); // LED einschalten
            Serial.printf("SMS loeschen");
            dash.data.DeleteSMS = false;
            GSM.delAllSms(); // this is optional
            // digitalWrite(LED_PIN, HIGH); // LED ausschalten
        }

        if (dash.data.ReInit)
        {
            // digitalWrite(LED_PIN, LOW); // LED einschalten
            Serial.printf("ReInit durchführen");
            dash.data.ReInit  = false;
            GSMInit();
            // digitalWrite(LED_PIN, HIGH); // LED ausschalten
        }


        if (dash.data.CheckSMS)
        {
            Serial.printf("Eingegange SMS prüfen\n");
            dash.data.CheckSMS = false;
            byte index = GSM.checkForSMS();
            if(index != 0)
            {
                Serial.printf("SMS Empfangen an Index:%d\n",index);
                // digitalWrite(LED_PIN, LOW); // LED einschalten
                // textSms=GSM.readSms(1); //read the first sms
        
                // if (textSms.indexOf("OK")!=-1) //first we need to know if the messege is correct. NOT an ERROR
            
                String smstext = GSM.readSms(index);
                Serial.println("Native:"+smstext);
                String strNumber = GSM.getNumberSmsFromBuffer();
                String strDate = GSM.getSmDateFromBuffer();

                Serial.printf("Empfangene SMS:%s von:%s vom:%s\n",smstext.c_str(),strNumber.c_str(),strDate.c_str());
                DiagManager.PushDiagData(msgAll,"Empfangene SMS:%s von:%s vom:%s\n",smstext.c_str(),strNumber.c_str(),strDate.c_str());
                // GSM.delAllSms(); // this is optional
                // digitalWrite(LED_PIN, HIGH); // LED ausschalten
            }
        }

        if ((dash.data.SleepMode) && (GSM.getSleepMode() == false))
        {
            // digitalWrite(LED_PIN, LOW); // LED einschalten
            Serial.printf("SleepMode setzen");
            if (GSM.setSleepMode(true))
                DiagManager.PushDiagData(msgFehler,"Sleepmode setzen ok");
            else
                DiagManager.PushDiagData(msgFehler,"Sleepmode setzen fehlerhaft");
            // digitalWrite(LED_PIN, HIGH); // LED ausschalten
        }
        if ((!dash.data.SleepMode) && (GSM.getSleepMode() == true))
        {
            // digitalWrite(LED_PIN, LOW); // LED einschalten
            Serial.printf("SleepMode zurücksetzen");
            if (GSM.setSleepMode(false))
                DiagManager.PushDiagData(msgFehler,"Sleepmode zurücksetzen OK");
            else
                DiagManager.PushDiagData(msgFehler,"Sleepmode zurücksetzen fehlerhaft");

            // digitalWrite(LED_PIN, HIGH); // LED ausschalten
        }
        if ((GyroManager.isMotionDetected()) && (dash.data.GSMModulPower == false))
        {
            // digitalWrite(LED_PIN, LOW); // LED einschalten
            Serial.printf("Bewegung erkannt\n");
            dash.data.GSMModulPower = true;
            DiagManager.PushDiagData(msgAll,"Bewegung erkannt");
        }
        if (dash.data.StartMotionDetection)
        {
            // digitalWrite(LED_PIN, LOW); // LED einschalten
            Serial.printf("Bewegungserkennung starten\n");
            dash.data.StartMotionDetection = false;
            DiagManager.PushDiagData(msgAll,"Bewegungserkennung gestartet");
            GyroManager.ResetMotion();
            dash.data.GSMModulPower = false;
        }	


        if ((dash.data.GSMModulPower) && (m_bGSMPowerState == false))
        {
            digitalWrite(GSMPOWER_PIN, HIGH); // GSM-Modul einschalten
            Serial.printf("GSM-Modul eingeschaltet\n");
            DiagManager.PushDiagData(msgFehler,"GSM-Modul eingeschaltet");
            m_bGSMPowerState = true;
            GSM.WaitForSMSCallReady();
            GSMInit();
        }
        else  if ((dash.data.GSMModulPower == false) && (m_bGSMPowerState == true))

        {
            digitalWrite(GSMPOWER_PIN, LOW); // GSM-Modul abschalten
            Serial.printf("GSM-Modul abgeschaltet\n");
            m_bGSMPowerState = false;
            DiagManager.PushDiagData(msgFehler,"GSM-Modul abgeschaltet");
        }
        if (dash.data.GotoDeepSleep)
        {
            digitalWrite(LED_PIN, LOW); // LED einschalten
            Serial.printf("Modul für 2 Sek in Deep Sleep setzen");
            dash.data.GotoDeepSleep  = false;
            // digitalWrite(LED_PIN, HIGH); // LED ausschalten
            ESP.deepSleep(0); // 4 Sekunden
        }


    }
    GSM.ReadGSMData();
}

  