#ifndef BUS_DASHBOARD_DEPARTURE_H
#define BUS_DASHBOARD_DEPARTURE_H


#include <WString.h>
#include <ctime>
#include <string>

using namespace std;

class Departure {
public:
    time_t departureDateTime;
    int departureDelay;
    int leavesInMins;

    Departure(const String& departureDateTime, String departureDelay) {
        struct tm rawDT = {0};
        strptime(departureDateTime.c_str(), "%Y-%m-%d %T", &rawDT);
        time_t parsedDT = mktime(&rawDT);
        this->departureDateTime = parsedDT;

        departureDelay.remove(0, 1); // remove plus
        this->departureDelay = strtol(departureDelay.c_str(), nullptr, 10);

        this->leavesInMins = 0;
    }

    string formatLeavesInMins() {
        if (leavesInMins == 0) {
            return string("< 1 min");
        }
        if (leavesInMins == 1) {
            return string("in einer Minute");
        }
        return string("in ") + String(leavesInMins).c_str() + string(" Minuten");
    }
};


#endif //BUS_DASHBOARD_DEPARTURE_H
