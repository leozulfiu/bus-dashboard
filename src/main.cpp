#include <Arduino.h>
#include <epd.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define TESTING true
const char* ssid = "***";
const char* password = "***";

typedef struct {
  time_t *departureDateTime;
  int departureDelay;
  int leavesInMins;
} Departure;

typedef struct {
  Departure *departures[3];
} NextDepartures;


NextDepartures *fetch_departure_times(String from, String to);
NextDepartures *fetch_merged_times(String from, String toDestinationA, String toDestinationB);
NextDepartures *merge_departures(NextDepartures *toDestinationA, NextDepartures *toDestinationB);
time_t fetchCurrentTime(String timezone);
void draw_time_values(NextDepartures **allData, time_t currentTime);
void calculateArrivalTimes(NextDepartures **allData, time_t currentTime);
String encode_as_URLParam(String input);
void initialize_wifi();
String formatTime(time_t *theTime);
String formatLeavesInMins(int delayInMinutes);
void logMessage(String message);

// -----------------------------------------------------------------------

void setup(void) {
  epd_init(2, 3);
  epd_wakeup(3);
  epd_set_memory(MEM_NAND);
  epd_clear();

  initialize_wifi();

  NextDepartures *allDepartureData[] = {
    fetch_departure_times("Zürich, Schumacherweg", "Zürich, Triemlispital"),
    fetch_departure_times("Zürich, Schumacherweg", "Zürich, Bahnhof Oerlikon Nord"),
    fetch_departure_times("Zürich, Glaubtenstrasse", "Zürich, Strassenverkehrsamt"),
    fetch_departure_times("Zürich, Glaubtenstrasse", "Zürich, Holzerhurd"),
    fetch_departure_times("Zürich, Glaubtenstrasse", "Zürich, Schwamendingerplatz"),
    fetch_merged_times("Zürich, Glaubtenstrasse", "Zürich, Waidhof", "Zürich, Mühlacker")
  };

  time_t currentTime = fetchCurrentTime("Europe/Zurich");

  calculateArrivalTimes(allDepartureData, currentTime);
  
  draw_time_values(allDepartureData, currentTime);
  
  epd_udpate();
  ESP.deepSleep(0);
  // shutdown
}

void loop(void) { }

// -----------------------------------------------------------------------

NextDepartures *fetch_departure_times(String from, String to) {
  HTTPClient http;
  String url = "http://bananas.cloud.tyk.io/transport?from="+ encode_as_URLParam(from) +"&to="+ encode_as_URLParam(to) +"&num=3&show_delays=1";
  http.begin(url);
  
  NextDepartures *nextDepartures = (NextDepartures*) malloc(sizeof(NextDepartures));
  
  if (http.GET() > 0) {
    String jsonPayload = http.getString();
    const size_t capacity = JSON_ARRAY_SIZE(5) + JSON_OBJECT_SIZE(1) + 5*JSON_OBJECT_SIZE(4) + 550;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, jsonPayload);

    JsonArray connections = doc["connections"];
    int maxConnections = 3;
    for (int i = 0; i < connections.size(); i++) {
      if (i > maxConnections) {
        // The API can sometimes return more or less than the maximum values of connections we want
        break;
      }
      nextDepartures->departures[i] = (Departure*) malloc(sizeof(Departure));
      JsonObject connection = connections[i];
      String departureDateTime = connection["departure"];
      String departureDelay = connection["departure_delay"];

      // Example: 2019-08-15 18:17:00
      nextDepartures->departures[i]->departureDateTime = (time_t*) malloc(sizeof(time_t));
      struct tm rawDT = {0};
      strptime(departureDateTime.c_str(), "%Y-%m-%d %T", &rawDT);
      time_t parsedDT = mktime(&rawDT);
      memcpy(nextDepartures->departures[i]->departureDateTime, &parsedDT, sizeof(time_t));

      departureDelay.remove(0, 1); // remove plus
      nextDepartures->departures[i]->departureDelay = strtol(departureDelay.c_str(), nullptr, 10);
    }
  } else {
    logMessage("Error while fetching.");
    // Blink LED or something
  }
  http.end();
  return nextDepartures;
}

NextDepartures *fetch_merged_times(String from, String toDestinationA, String toDestinationB) {
  NextDepartures *departuresToDestinationA = fetch_departure_times(from, toDestinationA);
  NextDepartures *departuresToDestinationB = fetch_departure_times(from, toDestinationB);
  return merge_departures(departuresToDestinationA, departuresToDestinationB);
}

void calculateArrivalTimes(NextDepartures **allData, time_t currentTime) {
  for(int i = 0; i < 6; i++) {
    for(int j = 0; j < 3; j++) {
      double diffSeconds = difftime(*allData[i]->departures[j]->departureDateTime, currentTime);
      diffSeconds += allData[i]->departures[j]->departureDelay;
      int diffMinutes = (int) floor(diffSeconds / 60);
      
      if (diffMinutes <= 0) {
        allData[i]->departures[j]->leavesInMins = 0;
      } else {
        allData[i]->departures[j]->leavesInMins = diffMinutes;
      }
    }
  }
}

void draw_time_values(NextDepartures **allData, time_t currentTime) {
  epd_disp_bitmap("HEAD2.BMP", 0, 0);
  epd_disp_bitmap("HEAD1.BMP", 0, 188);
  epd_disp_bitmap("HEAD3.BMP", 0, 382);

  epd_set_color(BLACK, WHITE);
  epd_set_en_font(ASCII32);

  int offsetXFirstColumn = 22, offsetXSecondColumn = 403, offsetXDelay = 138;
  int rowSpace = 35;
  
  for (int i = 0; i < 6; i += 2) {
    for (int j = 0; j < 3; j++) {
      int originY = (i/2 * 190) + 95;
      
      //first column
      epd_disp_string(formatTime(allData[i]->departures[j]->departureDateTime).c_str(), offsetXFirstColumn, originY + (j * rowSpace));
      epd_disp_string(formatLeavesInMins(allData[i]->departures[j]->leavesInMins).c_str(), offsetXFirstColumn + offsetXDelay, originY + (j * rowSpace));

      //second column
      epd_disp_string(formatTime(allData[i+1]->departures[j]->departureDateTime).c_str(), offsetXSecondColumn, originY + (j * rowSpace));
      epd_disp_string(formatLeavesInMins(allData[i+1]->departures[j]->leavesInMins).c_str(), offsetXSecondColumn + offsetXDelay, originY + (j * rowSpace));
    }
  }

  epd_set_en_font(ASCII48);
  epd_disp_string(formatTime(&currentTime).c_str(), 670, 5);
}

int sort_ascending(const void *a, const void *b) {
  Departure *ia = *(Departure **) a;
  Departure *ib = *(Departure **) b;

  // TODO: Implement time compare function instead of relying on string comparision
  return strcmp(formatTime(ia->departureDateTime).c_str(), formatTime(ib->departureDateTime).c_str());
}

NextDepartures *merge_departures(NextDepartures *toDestinationA, NextDepartures *toDestinationB) {
  Departure *mergedDepartures[6];

  // TODO: Can be probably simplified
  mergedDepartures[0] = toDestinationA->departures[0];
  mergedDepartures[1] = toDestinationA->departures[1];
  mergedDepartures[2] = toDestinationA->departures[2];
  mergedDepartures[3] = toDestinationB->departures[0];
  mergedDepartures[4] = toDestinationB->departures[1];
  mergedDepartures[5] = toDestinationB->departures[2];

  int array_length = sizeof(mergedDepartures) / sizeof(mergedDepartures[0]);
  qsort(mergedDepartures, array_length, sizeof(mergedDepartures[0]), sort_ascending);

  NextDepartures *mergedNextDepartures = (NextDepartures*) malloc(sizeof(NextDepartures));
  for (int i = 0; i < 3; i++) {
    mergedNextDepartures->departures[i] = (Departure*) malloc(sizeof(Departure));
    mergedNextDepartures->departures[i]->departureDateTime = mergedDepartures[i]->departureDateTime;
    mergedNextDepartures->departures[i]->departureDelay = mergedDepartures[i]->departureDelay;
  }
  
  return mergedNextDepartures;
}

time_t fetchCurrentTime(String timezone) {
  HTTPClient http;
  String url = "http://worldtimeapi.org/api/timezone/" + timezone;
  http.begin(url);
  time_t currentTime = {0};
  
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
    logMessage("Error while fetching.");
    // Blink LED or something
  }
  http.end();
  return currentTime;
}

void initialize_wifi() {
  // TODO: Can be split up in triggering the connection and waiting
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    logMessage("Connecting...");
  }
}

void logMessage(String message) {
  #ifdef TESTING
    Serial.println(message);
  #endif
}

String formatTime(time_t *theTime) {
  String hour = String(localtime(theTime)->tm_hour);
  String minutes = String(localtime(theTime)->tm_min);
  if (hour.length() == 1) {
    hour = String("0" + hour);
  }
  if (minutes.length() == 1) {
    minutes = String("0" + minutes);
  }
  return String(hour + ":" + minutes);
}

String formatLeavesInMins(int leavesInMins) {
  if (leavesInMins == 0) {
    return String("< 1 min");
  }
  if (leavesInMins == 1) {
     return String("in einer Minute");
  }
  return String("in ") + String(leavesInMins) + String(" Minuten");
}

// Copied from https://github.com/zenmanenergy/ESP8266-Arduino-Examples/blob/master/helloWorld_urlencoded/urlencode.ino
String encode_as_URLParam(String input) {
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < input.length(); i++){
      c=input.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;   
}
