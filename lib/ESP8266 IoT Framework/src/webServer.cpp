#include "webServer.h"

#include <FirebaseJson.h>
#include <Arduino.h>

#include "FS.h"
#include <LittleFS.h>

// #define LITTLEFS LittleFS 

// #ifdef ESP32
//     #include "LITTLEFS.h"
//     #define LittleFS LITTLEFS
// #elif defined(ESP8266)
//     #include "LittleFS.h"
// #endif


// Include the header file we create with webpack
#include "generated/html.h"

//Access to other classes for GUI functions
#include "WiFiManager.h"
#include "configManager.h"
#include "updater.h"
#include "dashboard.h"
#include "DiagManager.h"
// #include "OTAManager.h"


// AsyncWebSocket ws("/ws");
//AsyncWebServer server(80);


void webServer::begin(WebserverGetLibraryVersionCallback getLibraryVersionCallback)
{
#ifdef ESP32
    // TODO: Remove enter/exit traces after ESP32 build stable.  Experienced frequent crashing
    // during initial port when TCP stack not initialized before starting webserver instance.
    Serial.println(PSTR("webServer::begin enter"));
#endif
    m_getLibraryVersionCallback = getLibraryVersionCallback;
    //to enable testing and debugging of the interface
    DefaultHeaders::Instance().addHeader(PSTR("Access-Control-Allow-Origin"), PSTR("*"));

    server.serveStatic("/download", LittleFS, "/");

    server.onNotFound(requestHandler);

    //handle uploads
    server.on(PSTR("/upload"), HTTP_POST, [](AsyncWebServerRequest *request) {}, handleFileUpload);

    bindAll();

#ifdef ESP32
    Serial.println(PSTR("Calling server.begin();  Will fail on ESP32 if tcp stack not initialized.  Ensure WiFiManager was called first."));
#endif

    server.addHandler(&ws);
    server.begin();

#ifdef ESP32
   Serial.println(PSTR("webServer::begin done."));
#endif
}

void webServer::WifiGetResult(String& JSON)
{
    FirebaseJson jsonBuffer;
    // Serial.printf("Get Wifidata ActiveScan:%d Scan:%d\n",WiFiManager.m_bNetworkScan,bScan);

    jsonBuffer.set("captivePortal",WiFiManager.isCaptivePortal());
    jsonBuffer.set("ssid",WiFiManager.SSID());
    jsonBuffer.set("strength",String(WiFiManager.RSSI()));
    JSON = jsonBuffer.raw();
}

void webServer::bindAll()
{
    //Restart the ESP
    server.on(PSTR("/api/restart"), HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, PSTR("text/html"), ""); //respond first because of restart
        ESP.restart();
    });

    //update WiFi details
    server.on(PSTR("/api/wifi/set"), HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, PSTR("text/html"), ""); //respond first because of wifi change
        WiFiManager.setNewWifi(request->arg("ssid"), request->arg("pass"));
    });

    //update WiFi details with static IP
    server.on(PSTR("/api/wifi/setStatic"), HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, PSTR("text/html"), ""); //respond first because of wifi change
        WiFiManager.setNewWifi(request->arg("ssid"), request->arg("pass"), request->arg("ip"), request->arg("sub"), request->arg("gw"), request->arg("dns"));
    });

    //update WiFi details
    server.on(PSTR("/api/wifi/forget"), HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, PSTR("text/html"), ""); //respond first because of wifi change
        WiFiManager.forget();
    });

    //get WiFi details
    server.on(PSTR("/api/wifi/get"), HTTP_GET, [](AsyncWebServerRequest *request) {
        String JSON;
        GUI.WifiGetResult(JSON);
        Serial.printf("Wifi Get response:%s<-Ende\n",JSON.c_str());
        request->send(200, PSTR("text/html"), JSON);
    });


     //get diagnosedaten
    server.on(PSTR("/api/diagnosticdata/get"), HTTP_GET, [](AsyncWebServerRequest *request) {
        String JSON;
        FirebaseJson jsonBuffer;
        Serial.printf("Get Diagdata\n");

        FirebaseJsonArray files;// = jsonBuffer.createNestedArray("files");
        FirebaseJsonArray variablenames;//  = jsonBuffer.createNestedArray("variablenames");
        FirebaseJsonArray variablevalues;//  = jsonBuffer.createNestedArray("variablevalues");

        //get diagnosticdata listing
        for (int i = 0;i < DiagManager.GetLogSize();i++)
        {
            String str;
            str = DiagManager.GetLogString(i);
            files.add(str);
        }

        //Feste Werte ins JSON buegeln
        // jsonBuffer["variabletxt1"] = String("ABC Bimbam");
        // jsonBuffer["variablevalue1"] = String("ABC");
        // jsonBuffer["variabletxt2"] = String("Testvariable");
        // jsonBuffer["variablevalue2"] = String("DEF");
        // jsonBuffer["variabletxt3"] = String("Variable 3");
        // jsonBuffer["variablevalue3"] = String("123");

        //Hier gehts weiter die Dataen muessen besetzt werden 
        // for (int i = 0;i < 4;i++)
        // {
        //     String str;
        //     str = "Variable" + String(i);
        //     variablenames.add(str);
        //     str = "Wert" + String((i+1)*10);
        //     variablevalues.add(str);
        // }

        for (int i = 0;i < DiagManager.GetVariableMonitorSize();i++)
        {
            variablenames.add(DiagManager.GetVariableMonitorName(i));
            variablevalues.add(DiagManager.GetVariableMonitorValue(i));
        }
        jsonBuffer.add("files",files);
        jsonBuffer.add("variablenames",variablenames);
        jsonBuffer.add("variablevalues",variablevalues);
        JSON = jsonBuffer.raw();
        // serializeJson(jsonBuffer, JSON);
        // Serial.printf("Diagdata:%s<-Ende\n",JSON.c_str());
        request->send(200, PSTR("text/html"), JSON);

    });

    //get firmwareurl
    server.on(PSTR("/api/firmwareurl/get"), HTTP_GET, [](AsyncWebServerRequest *request) {
        GUI.m_LibraryVersionInfo.request = request;
        GUI.m_getLibraryVersionCallback(GUI.m_LibraryVersionInfo);
        // String JSON;
        // StaticJsonDocument<1000> jsonBuffer;
        // jsonBuffer["firmwareurl"] = String(configManager.data.FirmwareURL); 
        // serializeJson(jsonBuffer, JSON);
        // Serial.printf("api/firmwareurl/get:%s<-Ende\n",JSON.c_str());
        // request->send(200, PSTR("text/html"), JSON);
    });
    //do Firmwareupdate via firmwareurl 
    server.on(PSTR("/api/firmwareurl/DoUpdate"), HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, PSTR("text/html"), ""); //respond first because of restart
        Serial.printf("Anforderung von FirmwareUpdate\n");
        // OTAManager.m_bDoUpdate = true;
    });

    //get file listing
    server.on(PSTR("/api/files/get"), HTTP_GET, [](AsyncWebServerRequest *request) {

        // Serial.printf("/api/files/get Abfrage FileListing\n");
        String JSON;
        FirebaseJson  jsonBuffer;
        FirebaseJsonArray files;// = jsonBuffer.createNestedArray("files");

#ifdef ESP32
        //get file listing

        // 'Dir' and 'LittleFS.openDir' not implemented in ESP32, instead dir and 
        // files are represented as 'File'.  See docs/example:
        // https://github.com/lorol/ESPAsyncWebServer/blob/8c77d0e63f55160953fda843baa11435b05ae0bd/src/SPIFFSEditor.cpp#L187
        File dir = LittleFS.open("/");
        File entry = dir.openNextFile();
        
        // Copy enumerated filenames to temp array until serialized.  Reason... String mem refs 
        // returned by entry.name() could be reused, resulting in garbage chars returned to clients.
        StringArray strFiles;

        while (entry)
        {
            String fileName = String(entry.name());
            strFiles.add(fileName);
            files.add((strFiles.nth(strFiles.length() - 1))->c_str());
            entry = dir.openNextFile();
        }
#elif defined(ESP8266)
        //get file listing
        Dir dir = LittleFS.openDir("");
        while (dir.next())
            files.add(dir.fileName());
#endif

#ifdef ESP32
        //get used and total data

        // NOTE: esp_littlefs_info called twice, paying file block traversal each time.  Meh, that's
        // OK for small # of files.
        jsonBuffer.set("used",String(LittleFS.usedBytes())); 
        jsonBuffer.set("max",String(LittleFS.totalBytes()));
#elif defined(ESP8266)
        //get used and total data
        FSInfo fs_info;
        LittleFS.info(fs_info);
        jsonBuffer.set("used",String(fs_info.usedBytes));
        jsonBuffer.set("max",String(fs_info.totalBytes));
#endif
        jsonBuffer.add("files",files);
        JSON = jsonBuffer.raw();
        // Serial.printf("JSON:%s\n",JSON.c_str());
        // serializeJson(jsonBuffer, JSON);

#ifdef ESP32
        strFiles.free();
#endif

        request->send(200, PSTR("text/html"), JSON);
    });

    //remove file
    server.on(PSTR("/api/files/remove"), HTTP_POST, [](AsyncWebServerRequest *request) {

#ifdef ESP32
        String filePath = "/";
        filePath += request->arg("filename");
        LittleFS.remove(filePath.c_str());
        request->send(200, PSTR("text/html"), "");
#elif defined(ESP8266)
        LittleFS.remove("/" + request->arg("filename"));
        request->send(200, PSTR("text/html"), "");
#endif
    });

    //update from LittleFS
    server.on(PSTR("/api/update"), HTTP_POST, [](AsyncWebServerRequest *request) {        
        updater.requestStart(String("/") + request->arg("filename"));
        request->send(200, PSTR("text/html"), "");
    });

    //update status
    server.on(PSTR("/api/update-status"), HTTP_GET, [](AsyncWebServerRequest *request) {
        String JSON;
        FirebaseJson jsonBuffer;

        jsonBuffer.set("status",updater.getStatus());
        JSON = jsonBuffer.raw();
        // serializeJson(jsonBuffer, JSON);

        request->send(200, PSTR("text/html"), JSON);
    });

    //send binary configuration data
    server.on(PSTR("/api/config/get"), HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream(PSTR("application/octet-stream"));
        response->write(reinterpret_cast<char*>(&configManager.data), sizeof(configManager.data));
        request->send(response);
    });

    //receive binary configuration data from body
    server.on(
        PSTR("/api/config/set"), HTTP_POST,
        [this](AsyncWebServerRequest *request) {},
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {},
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            
            static uint8_t buffer[sizeof(configManager.data)];
            static uint32_t bufferIndex = 0;

            for (size_t i = 0; i < len; i++)
            {
                buffer[bufferIndex] = data[i];
                bufferIndex++;
            }

            if (index + len == total)
            {
                bufferIndex = 0;
                configManager.saveRaw(buffer);
                request->send(200, PSTR("text/html"), "");
            }

        });

    //receive binary configuration data from body
    server.on(
        PSTR("/api/dash/set"), HTTP_POST,
        [this](AsyncWebServerRequest *request) {},
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {},
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            // Serial.printf("Set Dashdata start:%d Laenge:%d\n",(request->arg("start")).toInt(),(request->arg("length")).toInt());
            memcpy(reinterpret_cast<uint8_t *>(&(dash.data)) + (request->arg("start")).toInt(), data, (request->arg("length")).toInt());
            request->send(200, PSTR("text/html"), "");
            // Serial.printf("inputwitch:%d \n",dash.data.inputswitch);
            // Serial.printf("inputbutton:%d \n",dash.data.inputbutton);

        });
}

// Callback for the html
void webServer::serveProgmem(AsyncWebServerRequest *request)
{    
    // Dump the byte array in PROGMEM with a 200 HTTP code (OK)
    AsyncWebServerResponse *response = request->beginResponse(200, PSTR("text/html"), html, html_len);

    // Tell the browswer the content is Gzipped
    response->addHeader(PSTR("Content-Encoding"), PSTR("gzip"));
    
    request->send(response);    
}

void webServer::handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    static File fsUploadFile;

    if (!index)
    {
        if (!filename.startsWith("/"))
            filename = "/" + filename;

        Serial.print(PSTR("Start file upload, filename="));
        Serial.println(filename);

        fsUploadFile = LittleFS.open(filename, "w");

#ifdef ESP32
        // Open write fail?
        if (!LittleFS.exists(filename))
        {
            Serial.println(PSTR("[E] LittleFS.open() failed.  Is LittleFS initialized?  Was updater.begin() called?"));
        }
#endif

    }

    for (size_t i = 0; i < len; i++)
    {
        fsUploadFile.write(data[i]);
    }

    if (final)
    {
        String JSON;
        FirebaseJson jsonBuffer;
#ifdef ESP32
        if (!filename.startsWith("/"))
            filename = "/" + filename;

        jsonBuffer.set("success",LittleFS.exists(filename));
#elif defined(ESP8266)
        jsonBuffer.set("success",fsUploadFile.isFile());
#endif
        JSON = jsonBuffer.raw();
        // serializeJson(jsonBuffer, JSON);

        request->send(200, PSTR("text/html"), JSON);
        fsUploadFile.close();
    }
}

webServer GUI;