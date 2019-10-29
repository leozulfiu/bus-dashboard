#ifndef BUS_DASHBOARD_CONNECTIONINPUT_H
#define BUS_DASHBOARD_CONNECTIONINPUT_H


#include <string>

using namespace std;

class ConnectionInput {
public:
    ConnectionInput(const char *fromStation, const char *toStation) {
        this->fromStation = fromStation;
        this->toStation = toStation;
    }

    string fromAsEncoded() {
        return ConnectionInput::encode_as_URLParam(fromStation);
    }

    string toAsEncoded() {
        return ConnectionInput::encode_as_URLParam(toStation);
    }

private:
    string fromStation;
    string toStation;
    static string encode_as_URLParam(const string& input);
};


#endif //BUS_DASHBOARD_CONNECTIONINPUT_H
