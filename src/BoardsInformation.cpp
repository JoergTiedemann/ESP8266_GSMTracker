#include "BoardsInformation.h"

#include "configManager.h"
#include "FirebaseJson.h"
#include <core_esp8266_version.h>


/* use provided PLATFORMIO_USED_LIBRARIES macro */ 
/* place string in flash so that it doesn't use up RAM. 
*  but then also needs special handling of PROGMEM strings. */
const char used_libs[] PROGMEM = { PLATFORMIO_USED_LIBRARIES };
__FlashStringHelper* used_libs_str = (__FlashStringHelper *)used_libs; 

CBoardsInformation BoardInformation;



void CBoardsInformation::PrintBoardInformation()
{

  // Standardpartition 1,3MB Flash: board_build.partitions = default.csv
  // MinSPIFFS max App 1,9MB Flash: board_build.partitions = min_spiffs.csv

  Serial.printf("*************************************************************************************************************\n");
  Serial.printf("Boardinformation OTA Version\n");
  Serial.printf("*************************************************************************************************************\n");
  Serial.printf("ESP82656 Chip Id = %d \n", ESP.getChipId());
//   Serial.printf("This chip has %d cores\n", ESP.getChipCores());
//   Serial.printf("PSRam Size = %d\n", ESP.getPsramSize());
//   Serial.printf("Heap Size = %d\n", ESP.getgetHeapSize());
  Serial.printf("Free Heap Size = %d\n", ESP.getFreeHeap());
  Serial.printf("Flash Chip Size = %d\n", ESP.getFlashChipSize());
  Serial.printf("Flash Chip Speed = %d\n", ESP.getFlashChipSpeed());
  Serial.printf("Flash Chip Mode = %d\n", ESP.getFlashChipMode());
  Serial.printf("CPU Frequency = %d MHZ\n", ESP.getCpuFreqMHz());
  Serial.printf("SDK Version = %s\n", ESP.getSdkVersion());
  Serial.printf("Sketch Size = %d\n", ESP.getSketchSize());
  Serial.printf("Free Sketch Space = %d\n", ESP.getFreeSketchSpace());
  Serial.printf("\n");
//   Serial.printf("List of required Mainpartitions\n");
//   find_partition("System NVS Partition",ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
//   find_partition("System OTA Partition",ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_OTA, NULL);
//   find_partition("OTA/App Partition",ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
//   find_partition("OTA/App Partition",ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
//   find_partition("Filesystem Partition",ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
  Serial.printf("*************************************************************************************************************\n");
 
}


/* prints the string that contains all libraries. doesn't require you to know the specific macros. */
void CBoardsInformation::print_used_libraries() {
    Serial.println(F("***************************************************"));
    Serial.printf("SDK Version:%s\n",ESP.getSdkVersion());
    Serial.printf("Core Version:%s\n",ESP.getCoreVersion().c_str());
    Serial.printf("Framework_Adruino_Espressif8266:%d.%d.%d\n", esp8266::coreVersionMajor(), esp8266::coreVersionMinor(),esp8266::coreVersionRevision());
    Serial.printf("Platform:%s\n",PIO_PLATFORM_VERSION);
    Serial.println(F("Verwendete Libraries (PLATFORMIO_USED_LIBRARIES)"));
    // Serial.printf("SDK Version lowlevel:%s\n",esp_get_idf_version());
    Serial.println(used_libs_str);
    Serial.println(F("***************************************************"));
    // Serial.println(F("Framework:"));
    // Serial.println(F(PIO_PACKAGE_FRAMEWORK_ARDUINOESPRESSIF32_PKG_VERSION));
    // Serial.println(F(PIO_PLATFORM_VERSION_MINOR));
    // Serial.println(F(PIO_PLATFORM_VERSION_RELEASE));
    // Serial.printf("Platform:%s.%s.%s\n",PIO_PLATFORM_VERSION_MAJOR,PIO_PLATFORM_VERSION_MINOR,PIO_PLATFORM_VERSION_RELEASE);

    /* you can search through the string per strchr_P() functions etc */
    m_strPlatformVersion =PIO_PLATFORM_VERSION;
	m_strFrameworkVersion =String(esp8266::coreVersionMajor())+"."+String(esp8266::coreVersionMinor())+"."+String(esp8266::coreVersionRevision());//PIO_PACKAGE_FRAMEWORK_ARDUINOESPRESSIF32_DECODED_VERSION;
	m_strSDKVersion=ESP.getSdkVersion();
	m_strLibraryVersion=used_libs_str;
}

void CBoardsInformation::SendLibraryVersion( WebserverLibraryInfo LibraryVersionInfo)
{
        Serial.printf("SendLibraryVersion to Webserver\n");

        String JSON;
        // StaticJsonDocument<5000> jsonBuffer;
        FirebaseJson jsonBuffer;
        jsonBuffer.set("firmwareurl",String(configManager.data.FirmwareURL)); 
        jsonBuffer.set("platformversion",BoardInformation.m_strPlatformVersion); 
        jsonBuffer.set("frameworkversion",BoardInformation.m_strFrameworkVersion); 
        jsonBuffer.set("sdkversion",BoardInformation.m_strSDKVersion); 
        jsonBuffer.set("libversion",BoardInformation.m_strLibraryVersion); 
        JSON = jsonBuffer.raw();
        Serial.printf("api/firmwareurl/get:%s<-Ende\n",JSON.c_str());
        LibraryVersionInfo.request->send(200, PSTR("text/html"), JSON);
}

/* prints version of a library that we know is included. */
// void print_known_libray() {
//     //again uses F() to create a flash-string to save space on the poor Uno..
//     Serial.print(F("Adafruit GFX library version: "));
//     Serial.println(F(LIB_VERSION_ADAFRUIT_GFX_LIBRARY));
// }

