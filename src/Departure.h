#ifndef BUS_DASHBOARD_DEPARTURE_H
#define BUS_DASHBOARD_DEPARTURE_H


#include <WString.h>
#include <ctime>
#include <string>
#include <cmath>

using namespace std;

class Departure {
public:
    time_t departureDateTime;
    int departureDelay;
    int leavesInMins;

    Departure(const String &departureDateTime, String departureDelay, time_t currentTime) {
        struct tm rawDT = {0};
        strptime(departureDateTime.c_str(), "%Y-%m-%d %T", &rawDT);
        time_t parsedDT = mktime(&rawDT);
        this->departureDateTime = parsedDT;

        departureDelay.remove(0, 1); // remove plus
        this->departureDelay = strtol(departureDelay.c_str(), nullptr, 10);

        double diffSeconds = difftime(this->departureDateTime, currentTime);
        diffSeconds += this->departureDelay;
        int diffMinutes = (int) floor(diffSeconds / 60);

        if (diffMinutes <= 0) {
            this->leavesInMins = 0;
        } else {
            this->leavesInMins = diffMinutes;
        }
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

    String formatDepartureTime() {
        String hour = String(localtime(&departureDateTime)->tm_hour);
        String minutes = String(localtime(&departureDateTime)->tm_min);
        if (hour.length() == 1) {
            hour = String("0" + hour);
        }
        if (minutes.length() == 1) {
            minutes = String("0" + minutes);
        }
        return String(hour + ":" + minutes);
    }
};


#endif //BUS_DASHBOARD_DEPARTURE_H
