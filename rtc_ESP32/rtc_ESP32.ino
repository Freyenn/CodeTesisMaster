#include <ESP32Time.h>
ESP32Time rtc; 

const char* ssid = "BUAP_Estudiantes";
const char* password =  "f85ac21de4";



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
}

void loop() {
  // put your main code here, to run repeatedly:
 
  
  Serial.println(rtc.getTime("%d/%m/%Y %H:%M:%S"));
  delay(1000);
}
