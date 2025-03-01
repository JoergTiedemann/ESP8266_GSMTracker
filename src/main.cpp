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
// #include "OTAManager.h"


char buffer [80];

String strTime("No Time");
String strBootTime("");

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


void setup() 
{
    Serial.begin(115200);
    BoardInformation.PrintBoardInformation();
    BoardInformation.print_used_libraries();


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
    dash.begin(500);
    // OTAManager.begin();
    //ueberwachung immer einschalten nach Neustart -> nein wollen wir erstmal nicht
    dash.data.RuntimeMonitor = configManager.data.ZeitCheckInit;

    DiagManager.AddVariableToMonitor(0,String("Letzter Systemstart"),&strBootTime);
    DiagManager.AddVariableToMonitor(2,String("Sytemzeit"),&strTime);
    DiagManager.AddVariableToMonitor(3,String("Free Heap"),&m_FreeHeap);
    // DiagManager.AddVariableToMonitor(1,String("Temperatur"),&m_Temperatur);
    // DiagManager.AddVariableToMonitor(5,String("akt. Tageslaufzeit"),&m_aktuelleTagesLaufzeit);
    // DiagManager.AddVariableToMonitor(6,String("PumpeIsRunning"),&m_PumpeIsRunning);
    // DiagManager.AddVariableToMonitor(8,String("ComboMode"),&dash.data.mode);
    // DiagManager.AddVariableToMonitor(9,String("CfgTestCombo"),&configManager.data.TestCombo);
   
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

    //your code here
    //task A
    if (taskA.previous == 0 || (millis() - taskA.previous > taskA.rate))
    {
        taskA.previous = millis();
        if (dash.data.Pumpenzustand)
        {
   	        dash.data.aktuelleLaufzeit = millis() / 1000;
        }
        dash.data.Temperatur = -999;
    }
}

  