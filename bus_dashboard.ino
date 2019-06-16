#include <epd.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


// WiFi Parameters
const char* ssid = "***";
const char* password = "***";

typedef struct {
  char *values[3][2][3];
  // Busline, Direction, Values as string
} DepartureTimes;

void fetchDepartureTimes(DepartureTimes *times);
void drawTimeValues(DepartureTimes *times);

void setup(void) {
  epd_init(2, 3);
  epd_wakeup(3);
  epd_set_memory(MEM_NAND);

  epd_clear();
  
  DepartureTimes times;
  fetchDepartureTimes(&times);
  
  drawTimeValues(&times);
  epd_udpate();

  // shutdown
}

void loop(void) { }

// ----------------------------------------------------------------------

void fetchDepartureTimes(DepartureTimes *times) {
  const String oerlikon_name = "Zürich Oerlikon, Bahnhof Nord";
  const String triemli_name = "Zürich, Triemlispital";

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println("Connecting...");
  }

  HTTPClient http;
  http.begin("http://fahrplan.search.ch/api/stationboard.json?stop=Schumacherweg&limit=6");
  int httpCode = http.GET();
  if (httpCode > 0) {
    String jsonPayload = http.getString();
    const size_t capacity = JSON_ARRAY_SIZE(6) + 8*JSON_OBJECT_SIZE(4) + 6*JSON_OBJECT_SIZE(10) + 1260;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, jsonPayload);

    JsonArray connections = doc["connections"];
    int counterTriemli = 0, counterOerlikon = 0;
    for (int i = 0; i < connections.size(); i++) {
      JsonObject connection = connections[i];
      JsonObject connection_terminal = connection["terminal"];
      String timeOnly = connection["time"];
      timeOnly.remove(0, 11); // remove date
      timeOnly.remove(5, 3); // remove seconds
      
      if (connection_terminal["name"] == triemli_name) {
        times->values[0][0][counterTriemli] = (char*) malloc((timeOnly.length() + 1) * sizeof(char));
        strcpy(times->values[0][0][counterTriemli], timeOnly.c_str());
        counterTriemli++;
      } else if (connection_terminal["name"] == oerlikon_name) {
        times->values[0][1][counterOerlikon] = (char*) malloc((timeOnly.length() + 1) * sizeof(char));
        strcpy(times->values[0][1][counterOerlikon], timeOnly.c_str());
        counterOerlikon++;
      } else {
        Serial.println("Error while parsing.");
      }
    }
  } else {
    Serial.println("Error while fetching.");
  }
  Serial.println("end parsing");
  http.end();
}

void drawTimeValues(DepartureTimes *times) {
  epd_disp_bitmap("HEAD1.BMP", 0, 0);
  epd_disp_bitmap("HEAD2.BMP", 0, 188);
  epd_disp_bitmap("HEAD3.BMP", 0, 382);

  epd_set_color(BLACK, WHITE);
  epd_set_en_font(ASCII32);
  epd_disp_string("14:39          in 16 Minuten", 22, 95);
  epd_disp_string("14:49          in 26 Minuten", 22, 130);
  epd_disp_string("14:59          in 36 Minuten", 22, 165);

  epd_disp_string("14:39          in 16 Minuten", 403, 95);
  epd_disp_string("14:49          in 26 Minuten", 403, 130);
  epd_disp_string("14:59          in 36 Minuten", 403, 165);

  //draw triemli connections
  for (int i = 0; i < 3; i++) {
    epd_disp_string(times->values[0][0][i], 22, 285 + (i * 35));
  }
  //draw oerlikon connections
  for (int i = 0; i < 3; i++) {
    epd_disp_string(times->values[0][1][i], 403, 285 + (i * 35));
  }

  epd_disp_string("14:39          in 16 Minuten", 22, 480);
  epd_disp_string("14:49          in 26 Minuten", 22, 515);
  epd_disp_string("14:59          in 36 Minuten", 22, 550);

  epd_disp_string("14:39          in 16 Minuten", 403, 480);
  epd_disp_string("14:49          in 26 Minuten", 403, 515);
  epd_disp_string("14:59          in 36 Minuten", 403, 550);
  
/*
  epd_set_en_font(ASCII48);
  char timeBuffer[6];
  sprintf(timeBuffer,"%02u:%02u",(now.hour()), now.minute());
  epd_disp_string(timeBuffer, 670, 5);
*/
}
