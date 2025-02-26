#include "timeSync.h"

#ifdef ESP32

// TODO: Implement ESP32 version

#elif defined(ESP8266)

#include <TZ.h>
#include <coredecls.h>
#include <PolledTimeout.h>

#endif


void NTPSync::begin()
{
#ifdef ESP32
    setTime(PSTR("UTC0"), "0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org");
#elif defined(ESP8266)
    setTime(TZ_Etc_UTC, "0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org");
#endif
}

void NTPSync::begin(const char* tz)
{
    setTime(tz, "0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org");
}

void NTPSync::begin(const char* tz, const char* server1, const char* server2, const char* server3)
{
    setTime(tz, server1, server2, server3);
}

bool NTPSync::isSynced()
{
    if (!synced)
    {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
            synced = true;
    }
    return synced; 
}

int8_t NTPSync::waitForSyncResult(unsigned long timeoutLengthMs)
{
#ifdef ESP32
    // Wait to become connected, or timeout expiration. 
    unsigned long timeout = timeoutLengthMs/10;
    // while((now = millis()) < timeout && now >= start)
    unsigned long  l = 0;
    while(l <  timeout )
    {
        delay(10);
        l++;
        struct tm timeinfo;
        if(getLocalTime(&timeinfo))
        {
            Serial.printf("Sntp Server synced nach %dms\n",l*10);
            synced = true;
            return 0;
        }
    }
#elif defined(ESP8266)
    using esp8266::polledTimeout::oneShot;
    oneShot timeout(timeoutLengthMs);
    while(!timeout)
    {
        yield();
        if(synced)
        {
            return 0;
        }
    }
#endif
    Serial.printf("Sntp Server not synced\n");

    return -1;
}

void NTPSync::setTime(const char* tz, const char* server1, const char* server2, const char* server3)
{
#ifdef ESP32
    // configTzTime @ https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-time.c
    configTzTime(tz, server1, server2, server3);
       
    // Flag as complete, since ESP32 configTzTime executes synchronously.-> nein das ist definitiv nicht so wir muessen drauf warten das wir mit dem snp Server verbunden sind
    // synced = true;

#elif defined(ESP8266)
    settimeofday_cb([this](){ synced = true; });
    configTime(tz, server1, server2, server3);

#endif
}

NTPSync timeSync;
