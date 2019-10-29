#include "Departure.h"
#include <sstream>

Departure::Departure(const String &departureDateTime, String departureDelay, time_t currentTime) {
    struct tm rawDT = {0};
    strptime(departureDateTime.c_str(), "%Y-%m-%d %T", &rawDT);
    time_t parsedDT = mktime(&rawDT);
    this->departureDateTime = parsedDT;

    departureDelay.remove(0, 1); // remove plus
    this->departureDelay = stoi(departureDelay.c_str(), nullptr, 10);

    double diffSeconds = difftime(this->departureDateTime, currentTime);
    diffSeconds += this->departureDelay;
    int diffMinutes = (int) floor(diffSeconds / 60);

    if (diffMinutes <= 0) {
        this->leavesInMins = 0;
    } else {
        this->leavesInMins = diffMinutes;
    }
}

string Departure::formatLeavesInMins() {
    if (leavesInMins == 0) {
        return string("< 1 min");
    }
    if (leavesInMins == 1) {
        return string("in einer Minute");
    }
    return string("in ") + String(leavesInMins).c_str() + string(" Minuten");
}

string Departure::formatDepartureTime() {
    string hour = intToString(localtime(&departureDateTime)->tm_hour);
    string minutes = intToString(localtime(&departureDateTime)->tm_min);

    if (hour.length() == 1) {
        hour = string("0" + hour);
    }
    if (minutes.length() == 1) {
        minutes = string("0" + minutes);
    }
    return string(hour + ":" + minutes);
}

string Departure::intToString(int input) {
    return dynamic_cast<ostringstream *>(&(ostringstream() << input))->str();
}
