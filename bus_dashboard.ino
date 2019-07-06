#include <epd.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


// WiFi Parameters
const char* ssid = "***";
const char* password = "***";

typedef struct {
  char *departureTime[3];
  char *departureDelay[3];
} DepartureData;


DepartureData *fetchDepartureTimes(String from, String to);
void drawTimeValues(DepartureData *data1, DepartureData *data2);
String encodeAsURLParam(String input);
void initializeWifi();

void setup(void) {
  epd_init(2, 3);
  epd_wakeup(3);
  epd_set_memory(MEM_NAND);

  epd_clear();
  initializeWifi();
  
  DepartureData *line80ToTriemli = fetchDepartureTimes("Zürich, Schumacherweg", "Zürich, Triemlispital");
  DepartureData *line80ToOerlikon = fetchDepartureTimes("Zürich, Schumacherweg", "Zürich, Bahnhof Oerlikon Nord");
  
  drawTimeValues(line80ToTriemli, line80ToOerlikon);
  epd_udpate();

  // shutdown
}

void loop(void) { }

// ----------------------------------------------------------------------

DepartureData *fetchDepartureTimes(String from, String to) {  
  HTTPClient http;
  String url = "http://bananas.cloud.tyk.io/transport?from="+ encodeAsURLParam(from) +"&to="+ encodeAsURLParam(to) +"&num=3&show_delays=1";
  http.begin(url);
  DepartureData *departureData;
  departureData = (DepartureData*) malloc(sizeof(DepartureData));
  
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
      String timeOnly = connection["departure"];
      timeOnly.remove(0, 11); // remove date
      timeOnly.remove(5, 3); // remove seconds
      String departureDelay = connection["departure_delay"];
      
      departureData->departureTime[i] = (char*) malloc((timeOnly.length() + 1) * sizeof(char)); 
      strcpy(departureData->departureTime[i], timeOnly.c_str());

      departureData->departureDelay[i] = (char*) malloc((departureDelay.length() + 1) * sizeof(char)); 
      strcpy(departureData->departureDelay[i], departureDelay.c_str());
    }
  } else {
    Serial.println("Error while fetching.");
  }
  http.end();
  return departureData;
}

void drawTimeValues(DepartureData* data1, DepartureData* data2) {
  epd_disp_bitmap("HEAD2.BMP", 0, 0);
  epd_disp_bitmap("HEAD1.BMP", 0, 188);
  epd_disp_bitmap("HEAD3.BMP", 0, 382);

  epd_set_color(BLACK, WHITE);
  epd_set_en_font(ASCII32);

  //draw triemli connections
  for (int i = 0; i < 3; i++) {
    epd_disp_string(data1->departureTime[i], 22, 95 + (i * 35));
    epd_disp_string(data1->departureDelay[i], 160, 95 + (i * 35));
  }
  //draw oerlikon connections
  for (int i = 0; i < 3; i++) {
    epd_disp_string(data2->departureTime[i], 403, 95 + (i * 35));
    epd_disp_string(data2->departureDelay[i], 541, 95 + (i * 35));
  }
  
  epd_disp_string("14:39          in 16 Minuten", 22, 285);
  epd_disp_string("14:49          in 26 Minuten", 22, 320);
  epd_disp_string("14:59          in 36 Minuten", 22, 355);

  epd_disp_string("14:39          in 16 Minuten", 403, 285);
  epd_disp_string("14:49          in 26 Minuten", 403, 320);
  epd_disp_string("14:59          in 36 Minuten", 403, 355);


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

void initializeWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
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
