#ifndef BUS_DASHBOARD_TIMEAPI_H
#define BUS_DASHBOARD_TIMEAPI_H


#include <ctime>
#include <ESP8266HTTPClient.h>
#include <WString.h>
#include <ArduinoJson.h>

class TimeApi {
public:
    static time_t fetchCurrentTime(const String &timezone);
};


#endif //BUS_DASHBOARD_TIMEAPI_H
