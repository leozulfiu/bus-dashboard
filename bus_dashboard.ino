#include <epd.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


// WiFi Parameters
const char* ssid = "***";
const char* password = "***";

typedef struct {
  char *departureTime;
  char *departureDelay;
} Departure;

typedef struct {
  Departure *departures[3];
} DepartureData;


DepartureData *fetch_departure_times(String from, String to);
void draw_time_values(DepartureData **allData);
DepartureData *merge_departures(DepartureData *destinationA, DepartureData *destinationB);
String encodeAsURLParam(String input);
void initialize_wifi();

// -----------------------------------------------------------------------

void setup(void) {
  epd_init(2, 3);
  epd_wakeup(3);
  epd_set_memory(MEM_NAND);

  epd_clear();
  initialize_wifi();

  DepartureData *allData[6];
  allData[0] = fetch_departure_times("Zürich, Schumacherweg", "Zürich, Triemlispital");
  allData[1] = fetch_departure_times("Zürich, Schumacherweg", "Zürich, Bahnhof Oerlikon Nord");

  allData[2] = fetch_departure_times("Zürich, Glaubtenstrasse", "Zürich, Strassenverkehrsamt");
  allData[3] = fetch_departure_times("Zürich, Glaubtenstrasse", "Zürich, Holzerhurd");

  allData[4] = fetch_departure_times("Zürich, Glaubtenstrasse", "Zürich, Schwamendingerplatz");

  DepartureData *waidhof = fetch_departure_times("Zürich, Glaubtenstrasse", "Zürich, Waidhof");
  DepartureData *muehlacker = fetch_departure_times("Zürich, Glaubtenstrasse", "Zürich, Mühlacker");
  allData[5] = merge_departures(waidhof, muehlacker);

  draw_time_values(allData);
  epd_udpate();

  // shutdown
}

void loop(void) { }

// -----------------------------------------------------------------------

DepartureData *fetch_departure_times(String from, String to) {
  Serial.println("Fetching: " + from + ", " + to);
  HTTPClient http;
  String url = "http://bananas.cloud.tyk.io/transport?from="+ encodeAsURLParam(from) +"&to="+ encodeAsURLParam(to) +"&num=3&show_delays=1";
  http.begin(url);
  DepartureData *departureData = (DepartureData*) malloc(sizeof(DepartureData));
  
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
      JsonObject connection = connections[i];
      String departureTime = connection["departure"];
      departureTime.remove(0, 11); // remove date
      departureTime.remove(5, 3); // remove seconds
      String departureDelay = connection["departure_delay"];

      departureData->departures[i] = (Departure*) malloc(sizeof(Departure));
      departureData->departures[i]->departureTime = (char*) malloc((departureTime.length() + 1) * sizeof(char)); 
      strcpy(departureData->departures[i]->departureTime, departureTime.c_str());

      departureData->departures[i]->departureDelay = (char*) malloc((departureDelay.length() + 1) * sizeof(char)); 
      strcpy(departureData->departures[i]->departureDelay, departureDelay.c_str());
    }
  } else {
    Serial.println("Error while fetching.");
  }
  http.end();
  return departureData;
}

void draw_time_values(DepartureData **allData) {
  int offsetXFirstColumn = 22;
  int offsetXSecondColumn = 403;
  
  int offsetYFirstBlock = 95;
  int offsetYSecondBlock = 285;
  int offsetYThirdBlock = 480;
  
  epd_disp_bitmap("HEAD2.BMP", 0, 0);
  epd_disp_bitmap("HEAD1.BMP", 0, 188);
  epd_disp_bitmap("HEAD3.BMP", 0, 382);

  epd_set_color(BLACK, WHITE);
  epd_set_en_font(ASCII32);

  //draw triemli connections
  for (int i = 0; i < 3; i++) {
    epd_disp_string(allData[0]->departures[i]->departureTime, offsetXFirstColumn, offsetYFirstBlock + (i * 35));
    epd_disp_string(allData[0]->departures[i]->departureDelay, offsetXFirstColumn + 138, offsetYFirstBlock + (i * 35));
  }
  //draw oerlikon connections
  for (int i = 0; i < 3; i++) {
    epd_disp_string(allData[1]->departures[i]->departureTime, offsetXSecondColumn, offsetYFirstBlock + (i * 35));
    epd_disp_string(allData[1]->departures[i]->departureDelay, offsetXSecondColumn + 138, offsetYFirstBlock + (i * 35));
  }

  //draw strassenverkehrsamt connections
  for (int i = 0; i < 3; i++) {
    epd_disp_string(allData[2]->departures[i]->departureTime, offsetXFirstColumn, offsetYSecondBlock + (i * 35));
    epd_disp_string(allData[2]->departures[i]->departureDelay, offsetXFirstColumn + 138, offsetYSecondBlock + (i * 35));
  }
  //draw holzerhurd connections
  for (int i = 0; i < 3; i++) {
    epd_disp_string(allData[3]->departures[i]->departureTime, offsetXSecondColumn, offsetYSecondBlock + (i * 35));
    epd_disp_string(allData[3]->departures[i]->departureDelay, offsetXSecondColumn + 138, offsetYSecondBlock + (i * 35));
  }

  //draw schwammendingerplatz connections
  for (int i = 0; i < 3; i++) {
    epd_disp_string(allData[4]->departures[i]->departureTime, offsetXFirstColumn, offsetYThirdBlock + (i * 35));
    epd_disp_string(allData[4]->departures[i]->departureDelay, offsetXFirstColumn + 138, offsetYThirdBlock + (i * 35));
  }
  //draw waidhof / muehlacker connections
  for (int i = 0; i < 3; i++) {
    epd_disp_string(allData[5]->departures[i]->departureTime, offsetXSecondColumn, offsetYThirdBlock + (i * 35));
    epd_disp_string(allData[5]->departures[i]->departureDelay, offsetXSecondColumn + 138, offsetYThirdBlock + (i * 35));
  }

/*
  epd_set_en_font(ASCII48);
  char timeBuffer[6];
  sprintf(timeBuffer,"%02u:%02u",(now.hour()), now.minute());
  epd_disp_string(timeBuffer, 670, 5);
*/
}

int sort_desc(const void *a, const void *b) {
  Departure *ia = *(Departure **) a;
  Departure *ib = *(Departure **) b;
  return strcmp(ia->departureTime, ib->departureTime);
}

DepartureData *merge_departures(DepartureData *destinationA, DepartureData *destinationB) {
  Departure *mergedDepartures[6];

  mergedDepartures[0] = destinationA->departures[0];
  mergedDepartures[1] = destinationA->departures[1];
  mergedDepartures[2] = destinationA->departures[2];
  mergedDepartures[3] = destinationB->departures[0];
  mergedDepartures[4] = destinationB->departures[1];
  mergedDepartures[5] = destinationB->departures[2];

  int lt_length = sizeof(mergedDepartures) / sizeof(mergedDepartures[0]);
  qsort(mergedDepartures, lt_length, sizeof(mergedDepartures[0]), sort_desc);

  DepartureData *mergedDepartureData = (DepartureData*) malloc(sizeof(DepartureData));

  for (int i = 0; i < 3; i++) {
    mergedDepartureData->departures[i] = (Departure*) malloc(sizeof(Departure));
    mergedDepartureData->departures[i]->departureTime = mergedDepartures[i]->departureTime;
    mergedDepartureData->departures[i]->departureDelay = mergedDepartures[i]->departureDelay;
  }
  
  return mergedDepartureData;
}

void initialize_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.println("Connecting...");
  }
}

// Copied from https://github.com/zenmanenergy/ESP8266-Arduino-Examples/blob/master/helloWorld_urlencoded/urlencode.ino
String encodeAsURLParam(String input) {
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
