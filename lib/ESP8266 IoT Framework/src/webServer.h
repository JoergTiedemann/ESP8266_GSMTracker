#ifndef SERVER_H
#define SERVER_H

#include <ESPAsyncWebServer.h>

typedef struct webserver_library_info_t
{
    AsyncWebServerRequest *request;
	String m_strPlatformVersion;
	String m_strFrameworkVersion;
	String m_strSDKVersion;
	String m_strLibraryVersion;

} WebserverLibraryInfo;

typedef void (*WebserverGetLibraryVersionCallback)(WebserverLibraryInfo); 

class webServer
{

private:    
    static void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    static void serveProgmem(AsyncWebServerRequest *request);
    void bindAll();

public:
    WebserverGetLibraryVersionCallback m_getLibraryVersionCallback = NULL;
    WebserverLibraryInfo m_LibraryVersionInfo; 
    void WifiGetResult(String& JSON);

    AsyncWebServer server;
    AsyncWebSocket ws;
    ArRequestHandlerFunction requestHandler = serveProgmem;
    void begin(WebserverGetLibraryVersionCallback getLibraryVersionCallback = NULL);
    webServer():server(80),ws("/ws"){};
};

extern webServer GUI;


#endif
