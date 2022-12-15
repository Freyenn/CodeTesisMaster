
#include <WiFi.h>
#include <ESP32Time.h>
#include "time.h"

//========================================================================
ESP32Time rtc; 
//WiFi Details
const char* ssid       = "BUAP_Estudiantes";
const char* password   = "f85ac21de4";

// NTP server to request time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -6*3600;
const int   daylightOffset_sec = 0;

const int push_b = 23; 
#define LED 2

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void synchroniseWith_NTP_Time() {
  initWiFi();
  struct tm timeinfo;
  time_t now; 
  digitalWrite(LED, HIGH); // Encender el LED
  Serial.print("configTime uses ntpServer ");
  Serial.println(ntpServer);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.print("synchronising time");
  
  while (timeinfo.tm_year + 1900 < 2000 ) {
    time(&now);                       // read the current time
    localtime_r(&now, &timeinfo);
    
    delay(100);
    Serial.print(".");
  }
  localtime_r(&now, &timeinfo);  
  int yr = timeinfo.tm_year + 1900;
  int mt = timeinfo.tm_mon + 1;
  int dy = timeinfo.tm_mday;
  int hr = timeinfo.tm_hour;
  int mi = timeinfo.tm_min;
  int se = timeinfo.tm_sec;
  rtc.setTime(se, mi, hr, dy, mt,yr);
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.print("\n time synchronsized \n");  
  digitalWrite(LED, LOW); // Encender el LED
}

void setup() {
  Serial.begin(115200);
  pinMode (LED, OUTPUT); // Cambia el PIN del led a OUTPUT
  //Sincronizar RTC con Tiempo Actual
  synchroniseWith_NTP_Time();
}

void loop() {
   
  Serial.println(rtc.getTime("%d/%m/%Y %H:%M:%S"));
  delay(1000);

}
