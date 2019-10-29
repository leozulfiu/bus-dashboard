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

    Departure(const String &departureDateTime, String departureDelay, time_t currentTime);

    string formatLeavesInMins();
    string formatDepartureTime();
    static string intToString(int input);
};


#endif //BUS_DASHBOARD_DEPARTURE_H
