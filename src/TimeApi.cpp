#include "TimeApi.h"

time_t TimeApi::fetchCurrentTime(const String &timezone) {
    HTTPClient http;
    String url = "http://worldtimeapi.org/api/timezone/" + timezone;
    http.begin(url);
    time_t currentTime = {0};;

    if (http.GET() > 0) {
        String jsonPayload = http.getString();
        const size_t capacity = JSON_OBJECT_SIZE(15) + 350;
        DynamicJsonDocument doc(capacity);
        deserializeJson(doc, jsonPayload);

        String currentTimestamp = doc["datetime"];
        //Example: 2019-08-15T17:28:12.798760+02:00
        currentTimestamp.remove(19, 13); // strip off microseconds and rest

        struct tm rawDT = {0};
        strptime(currentTimestamp.c_str(), "%Y-%m-%dT%T", &rawDT);
        currentTime = mktime(&rawDT);
    } else {
        http.end();
        // TODO: throw -1;
    }
    http.end();
    return currentTime;
}
